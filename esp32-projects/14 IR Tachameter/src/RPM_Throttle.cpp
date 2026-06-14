#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- ESC ----------
#define ESC_PIN 32

#define ESC_MIN_US 1000
#define ESC_MAX_US 2000

// Safe limits for first closed-loop test
#define ESC_SAFE_MAX_US 1400
#define ESC_STARTUP_US 1150

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1
#define TARGET_RPM 1700

#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

#define SIGNAL_LOST_TIMEOUT_MS 2000
#define RPM_ZERO_TIMEOUT_MS 6000

#define PERIOD_TOLERANCE 0.25

// ---------- Timing ----------
#define CONTROL_PERIOD_MS 100
#define MAX_THROTTLE_STEP_US 5

// ---------- Filtering ----------
const float rpmFilterAlpha = 0.15;

// ---------- Controller Gains ----------
float Kp = 0.03;
float Ki = 0.005;

// ---------- Objects ----------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo esc;

// ---------- Tach Interrupt Variables ----------
volatile unsigned long lastEdgeTimeUs = 0;
volatile unsigned long latestPeriodUs = 0;
volatile bool newPulseAvailable = false;
volatile unsigned long pulseCount = 0;
volatile unsigned long rejectedFastPulseCount = 0;

// ---------- Tach Main Variables ----------
float rpmRaw = 0.0;
float rpmFiltered = 0.0;

unsigned long lastGoodPeriodUs = 0;
unsigned long acceptedPulseCount = 0;
unsigned long rejectedOutlierCount = 0;
unsigned long lastAcceptedPulseMs = 0;

bool locked = false;
bool signalOK = false;

// ---------- Control Variables ----------
int throttleUs = ESC_MIN_US;
float integralError = 0.0;

// ---------- Interrupt ----------
void IRAM_ATTR rpmISR() {
  unsigned long nowUs = micros();

  if (lastEdgeTimeUs == 0) {
    lastEdgeTimeUs = nowUs;
    return;
  }

  unsigned long dt = nowUs - lastEdgeTimeUs;

  // Reject impossible fast pulses
  if (dt < MIN_PULSE_INTERVAL_US) {
    rejectedFastPulseCount++;
    return;
  }

  latestPeriodUs = dt;
  lastEdgeTimeUs = nowUs;
  pulseCount++;
  newPulseAvailable = true;
}

// ---------- Helper Functions ----------
bool isPeriodReasonable(unsigned long newPeriod, unsigned long referencePeriod) {
  if (referencePeriod == 0) {
    return true;
  }

  float lowerLimit = referencePeriod * (1.0 - PERIOD_TOLERANCE);
  float upperLimit = referencePeriod * (1.0 + PERIOD_TOLERANCE);

  return (newPeriod >= lowerLimit && newPeriod <= upperLimit);
}

int slewLimit(int currentValue, int targetValue, int maxStep) {
  if (targetValue > currentValue + maxStep) {
    return currentValue + maxStep;
  }

  if (targetValue < currentValue - maxStep) {
    return currentValue - maxStep;
  }

  return targetValue;
}

void updateOLED(float rpm, int target, int throttle, bool sigOK, bool isLocked) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("RPM Control Test");

  display.setCursor(0, 12);
  display.print("RPM: ");
  display.println(rpm, 0);

  display.setCursor(0, 24);
  display.print("Target: ");
  display.println(target);

  display.setCursor(0, 36);
  display.print("ESC: ");
  display.print(throttle);
  display.println(" us");

  display.setCursor(0, 48);
  display.print("Sig:");
  display.print(sigOK ? "OK" : "NO");

  display.print(" Lock:");
  display.print(isLocked ? "Y" : "N");

  display.setCursor(0, 56);
  display.print("Kp:");
  display.print(Kp, 3);
  display.print(" Ki:");
  display.print(Ki, 3);

  display.display();
}

void armESC() {
  Serial.println("Arming ESC at minimum throttle...");
  esc.writeMicroseconds(ESC_MIN_US);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Arming ESC...");
  display.setCursor(0, 16);
  display.println("Throttle 1000us");
  display.display();

  delay(4000);

  Serial.println("ESC armed.");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("OLED failed to initialize");
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  // ESC setup
  esc.setPeriodHertz(50);
  esc.attach(ESC_PIN, ESC_MIN_US, ESC_MAX_US);

  throttleUs = ESC_MIN_US;
  esc.writeMicroseconds(throttleUs);

  delay(1000);
  armESC();

  attachInterrupt(digitalPinToInterrupt(IR_PIN), rpmISR, FALLING);

  Serial.println("==================================");
  Serial.println("RPM Control Test Started");
  Serial.println("Sensor target: Noctua fan");
  Serial.print("Target RPM: ");
  Serial.println(TARGET_RPM);
  Serial.print("ESC safe max: ");
  Serial.println(ESC_SAFE_MAX_US);
  Serial.print("ESC startup: ");
  Serial.println(ESC_STARTUP_US);
  Serial.println("==================================");
}

void loop() {
  static unsigned long lastDisplayMs = 0;
  static unsigned long lastSerialMs = 0;
  static unsigned long lastControlMs = 0;
  static unsigned long lastPlotMs = 0;

  unsigned long nowMs = millis();

  // ---------- Copy interrupt variables ----------
  noInterrupts();
  bool pulseAvailableCopy = newPulseAvailable;
  unsigned long periodCopy = latestPeriodUs;
  unsigned long fastRejectedCopy = rejectedFastPulseCount;
  newPulseAvailable = false;
  interrupts();

  // ---------- Process RPM pulse ----------
  if (pulseAvailableCopy && periodCopy > 0) {
    bool acceptPulse = false;

    if (acceptedPulseCount < 5) {
      acceptPulse = true;
    } else {
      acceptPulse = isPeriodReasonable(periodCopy, lastGoodPeriodUs);
    }

    if (acceptPulse) {
      lastGoodPeriodUs = periodCopy;
      acceptedPulseCount++;
      lastAcceptedPulseMs = nowMs;
      signalOK = true;

      if (acceptedPulseCount >= 5) {
        locked = true;
      }

      rpmRaw = (60.0 * 1000000.0) / (periodCopy * PULSES_PER_REV);

      if (rpmRaw <= MAX_VALID_RPM) {
        if (rpmFiltered <= 0.0) {
          rpmFiltered = rpmRaw;
        } else {
          rpmFiltered = (rpmFilterAlpha * rpmRaw) + ((1.0 - rpmFilterAlpha) * rpmFiltered);
        }
      }
    } else {
      rejectedOutlierCount++;
    }
  }

  // ---------- Signal status ----------
  if (lastAcceptedPulseMs > 0) {
    unsigned long timeSinceGoodPulseMs = nowMs - lastAcceptedPulseMs;

    if (timeSinceGoodPulseMs > SIGNAL_LOST_TIMEOUT_MS) {
      signalOK = false;
    }

    if (timeSinceGoodPulseMs > RPM_ZERO_TIMEOUT_MS) {
      rpmRaw = 0.0;
      rpmFiltered = 0.0;
      lastGoodPeriodUs = 0;
      acceptedPulseCount = 0;
      rejectedOutlierCount = 0;
      locked = false;
      signalOK = false;
      lastAcceptedPulseMs = 0;

      noInterrupts();
      lastEdgeTimeUs = 0;
      latestPeriodUs = 0;
      newPulseAvailable = false;
      pulseCount = 0;
      rejectedFastPulseCount = 0;
      interrupts();
    }
  } else {
    signalOK = false;
  }

  // ---------- Closed-loop PI control ----------
  if (nowMs - lastControlMs >= CONTROL_PERIOD_MS) {
    lastControlMs = nowMs;

    if (signalOK && locked) {
      float error = TARGET_RPM - rpmFiltered;

      // Deadband to prevent tiny hunting around target
      if (abs(error) < 20) {
        error = 0;
      }

      integralError += error * (CONTROL_PERIOD_MS / 1000.0);

      // Anti-windup
      integralError = constrain(integralError, -5000.0, 5000.0);

      float correction = (Kp * error) + (Ki * integralError);

      int desiredThrottle = throttleUs + correction;

      desiredThrottle = constrain(desiredThrottle, ESC_MIN_US, ESC_SAFE_MAX_US);

      throttleUs = slewLimit(throttleUs, desiredThrottle, MAX_THROTTLE_STEP_US);

      esc.writeMicroseconds(throttleUs);
    } else {
      // Startup mode:
      // Give a small safe throttle so the BLDC can spin,
      // while the sensor waits for valid RPM pulses.
      throttleUs = ESC_STARTUP_US;
      integralError = 0.0;
      esc.writeMicroseconds(throttleUs);
    }
  }

  // ---------- Normal Serial output ----------
  if (nowMs - lastSerialMs >= 250) {
    lastSerialMs = nowMs;

    Serial.print("Target: ");
    Serial.print(TARGET_RPM);

    Serial.print(" | RPM Raw: ");
    Serial.print(rpmRaw, 0);

    Serial.print(" | RPM Filt: ");
    Serial.print(rpmFiltered, 0);

    Serial.print(" | ESC us: ");
    Serial.print(throttleUs);

    Serial.print(" | Signal: ");
    Serial.print(signalOK ? "OK" : "NO SIG");

    Serial.print(" | Lock: ");
    Serial.print(locked ? "Y" : "N");

    Serial.print(" | FastReject: ");
    Serial.print(fastRejectedCopy);

    Serial.print(" | OutlierReject: ");
    Serial.println(rejectedOutlierCount);
  }

  // ---------- Teleplot output ----------
  if (nowMs - lastPlotMs >= 100) {
    lastPlotMs = nowMs;

    Serial.print(">target_rpm:");
    Serial.println(TARGET_RPM);

    Serial.print(">rpm:");
    Serial.println(rpmFiltered);

    Serial.print(">raw_rpm:");
    Serial.println(rpmRaw);

    Serial.print(">esc_us:");
    Serial.println(throttleUs);

    Serial.print(">error:");
    Serial.println(TARGET_RPM - rpmFiltered);

    Serial.print(">signal_ok:");
    Serial.println(signalOK ? 1 : 0);

    Serial.print(">lock:");
    Serial.println(locked ? 1 : 0);
  }

  // ---------- OLED output ----------
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;
    updateOLED(rpmFiltered, TARGET_RPM, throttleUs, signalOK, locked);
  }
}