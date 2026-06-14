#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// Your target is around 12,000 RPM.
// 15,000 RPM gives margin.
#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

// If signal disappears briefly, DO NOT immediately show 0.
// This prevents random 0 flashes.
#define SIGNAL_LOST_TIMEOUT_MS 2000

// Only after this much time with no signal do we show RPM = 0.
#define RPM_ZERO_TIMEOUT_MS 6000

// Outlier rejection done in loop, not ISR.
// 0.25 = accept periods within +/-25% of previous good period.
#define PERIOD_TOLERANCE 0.25

// Filtering amount.
const float filterAlpha = 0.15;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Interrupt Variables ----------
volatile unsigned long lastEdgeTimeUs = 0;
volatile unsigned long latestPeriodUs = 0;
volatile bool newPulseAvailable = false;
volatile unsigned long pulseCount = 0;
volatile unsigned long rejectedFastPulseCount = 0;

// ---------- Main Variables ----------
float rpmRaw = 0.0;
float rpmFiltered = 0.0;

unsigned long lastGoodPeriodUs = 0;
unsigned long acceptedPulseCount = 0;
unsigned long rejectedOutlierCount = 0;
unsigned long lastAcceptedPulseMs = 0;

bool locked = false;
bool signalOK = false;

// Keep ISR extremely short.
// Your sensor is active-low, so we count FALLING edge.
void IRAM_ATTR rpmISR() {
  unsigned long nowUs = micros();

  if (lastEdgeTimeUs == 0) {
    lastEdgeTimeUs = nowUs;
    return;
  }

  unsigned long dt = nowUs - lastEdgeTimeUs;

  // Reject impossible fast pulses.
  if (dt < MIN_PULSE_INTERVAL_US) {
    rejectedFastPulseCount++;
    return;
  }

  latestPeriodUs = dt;
  lastEdgeTimeUs = nowUs;
  pulseCount++;
  newPulseAvailable = true;
}

bool isPeriodReasonable(unsigned long newPeriod, unsigned long referencePeriod) {
  if (referencePeriod == 0) {
    return true;
  }

  float lowerLimit = referencePeriod * (1.0 - PERIOD_TOLERANCE);
  float upperLimit = referencePeriod * (1.0 + PERIOD_TOLERANCE);

  return (newPeriod >= lowerLimit && newPeriod <= upperLimit);
}

void updateOLED(float rpmFilt, float rpmRawValue, unsigned long periodUs, int irState,
                unsigned long accepted, unsigned long fastRejected,
                unsigned long outlierRejected, bool isLocked, bool sigOK) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Hovercraft Tach");

  display.setCursor(0, 10);
  display.print("RPM: ");
  display.println(rpmRawValue, 0);

  display.setCursor(0, 22);
  display.print("Filt:");
  display.print(rpmFilt, 0);

  display.setCursor(70, 22);
  display.print(sigOK ? "OK" : "NO SIG");

  display.setCursor(0, 34);
  display.print("T: ");
  display.print(periodUs);
  display.println(" us");

  display.setCursor(0, 46);
  display.print("IR:");
  display.print(irState == HIGH ? "H" : "L");
  display.print(" Lock:");
  display.print(isLocked ? "Y" : "N");

  display.setCursor(0, 56);
  display.print("A:");
  display.print(accepted);
  display.print(" F:");
  display.print(fastRejected);
  display.print(" O:");
  display.print(outlierRejected);

  display.display();
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
  display.println("Tach starting...");
  display.display();

  delay(1000);

  attachInterrupt(digitalPinToInterrupt(IR_PIN), rpmISR, FALLING);

  Serial.println("==================================");
  Serial.println("Safe ESP32 TCRT5000 Tach Started");
  Serial.println("Random zero flash protection enabled");
  Serial.print("MAX_VALID_RPM: ");
  Serial.println(MAX_VALID_RPM);
  Serial.print("MIN_PULSE_INTERVAL_US: ");
  Serial.println(MIN_PULSE_INTERVAL_US);
  Serial.println("==================================");
}

void loop() {
  static unsigned long lastDisplayMs = 0;
  static unsigned long lastSerialMs = 0;

  unsigned long nowMs = millis();
  unsigned long nowUs = micros();

  // Copy interrupt variables safely.
  noInterrupts();
  bool pulseAvailableCopy = newPulseAvailable;
  unsigned long periodCopy = latestPeriodUs;
  unsigned long lastEdgeCopy = lastEdgeTimeUs;
  unsigned long pulseCountCopy = pulseCount;
  unsigned long fastRejectedCopy = rejectedFastPulseCount;
  newPulseAvailable = false;
  interrupts();

  // Process new pulse in normal loop, not ISR.
  if (pulseAvailableCopy && periodCopy > 0) {
    bool acceptPulse = false;

    // Accept early pulses to establish lock.
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
          rpmFiltered = (filterAlpha * rpmRaw) + ((1.0 - filterAlpha) * rpmFiltered);
        }
      }
    } else {
      rejectedOutlierCount++;
    }
  }

  // Signal status logic.
  // Short signal loss: keep last RPM, show NO SIG if needed.
  // Long signal loss: then set RPM to zero.
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

      // Reset ISR timing only after a long confirmed stop/no signal.
      noInterrupts();
      lastEdgeTimeUs = 0;
      latestPeriodUs = 0;
      newPulseAvailable = false;
      pulseCount = 0;
      rejectedFastPulseCount = 0;
      interrupts();

      periodCopy = 0;
      pulseCountCopy = 0;
      fastRejectedCopy = 0;
      lastAcceptedPulseMs = 0;
    }
  } else {
    // Before first valid pulse.
    rpmRaw = 0.0;
    rpmFiltered = 0.0;
    signalOK = false;
  }

  int irState = digitalRead(IR_PIN);

  // Serial output.
  if (nowMs - lastSerialMs >= 250) {
    lastSerialMs = nowMs;

    Serial.print("RPM Raw: ");
    Serial.print(rpmRaw, 0);
    Serial.print(">rpmRaw:");

    Serial.print(" | RPM Filtered: ");
    Serial.print(rpmFiltered, 0);
    Serial.print(">rpmFiltered:");

    Serial.print(" | Period us: ");
    Serial.print(periodCopy);

    Serial.print(" | IR: ");
    Serial.print(irState == HIGH ? "HIGH" : "LOW");

    Serial.print(" | Signal: ");
    Serial.print(signalOK ? "OK" : "NO SIG");

    Serial.print(" | Lock: ");
    Serial.print(locked ? "Y" : "N");

    Serial.print(" | Accepted: ");
    Serial.print(acceptedPulseCount);

    Serial.print(" | FastReject: ");
    Serial.print(fastRejectedCopy);

    Serial.print(" | OutlierReject: ");
    Serial.println(rejectedOutlierCount);
  }

  // OLED update.
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    updateOLED(
      rpmFiltered,
      rpmRaw,
      periodCopy,
      irState,
      acceptedPulseCount,
      fastRejectedCopy,
      rejectedOutlierCount,
      locked,
      signalOK
    );
  }
}