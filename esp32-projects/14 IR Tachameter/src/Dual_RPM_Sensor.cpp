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

// ---------- Sensor Pins ----------
#define SENSOR1_PIN 27   // TCRT5000
#define SENSOR2_PIN 26   // New 3-wire IR sensor

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// Your target is about 12,000 RPM.
// 15,000 RPM gives margin.
#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

// Increase this to prevent quick false zero drops.
#define RPM_TIMEOUT_MS 2000

#define PERIOD_TOLERANCE 0.25

const float filterAlpha = 0.15;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Sensor 1 Interrupt Variables ----------
volatile unsigned long s1_lastEdgeTimeUs = 0;
volatile unsigned long s1_latestPeriodUs = 0;
volatile bool s1_newPulseAvailable = false;
volatile unsigned long s1_pulseCount = 0;
volatile unsigned long s1_fastRejectCount = 0;

// ---------- Sensor 2 Interrupt Variables ----------
volatile unsigned long s2_lastEdgeTimeUs = 0;
volatile unsigned long s2_latestPeriodUs = 0;
volatile bool s2_newPulseAvailable = false;
volatile unsigned long s2_pulseCount = 0;
volatile unsigned long s2_fastRejectCount = 0;

// ---------- Sensor State Struct ----------
struct TachState {
  float rpmRaw = 0.0;
  float rpmFiltered = 0.0;
  unsigned long lastGoodPeriodUs = 0;
  unsigned long acceptedPulseCount = 0;
  unsigned long outlierRejectCount = 0;
  bool locked = false;
};

TachState tach1;
TachState tach2;

// ---------- Interrupts ----------
void IRAM_ATTR sensor1ISR() {
  unsigned long nowUs = micros();

  if (s1_lastEdgeTimeUs == 0) {
    s1_lastEdgeTimeUs = nowUs;
    return;
  }

  unsigned long dt = nowUs - s1_lastEdgeTimeUs;

  if (dt < MIN_PULSE_INTERVAL_US) {
    s1_fastRejectCount++;
    return;
  }

  s1_latestPeriodUs = dt;
  s1_lastEdgeTimeUs = nowUs;
  s1_pulseCount++;
  s1_newPulseAvailable = true;
}

void IRAM_ATTR sensor2ISR() {
  unsigned long nowUs = micros();

  if (s2_lastEdgeTimeUs == 0) {
    s2_lastEdgeTimeUs = nowUs;
    return;
  }

  unsigned long dt = nowUs - s2_lastEdgeTimeUs;

  if (dt < MIN_PULSE_INTERVAL_US) {
    s2_fastRejectCount++;
    return;
  }

  s2_latestPeriodUs = dt;
  s2_lastEdgeTimeUs = nowUs;
  s2_pulseCount++;
  s2_newPulseAvailable = true;
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

void processTachPulse(TachState &tach, bool pulseAvailable, unsigned long periodUs) {
  if (!pulseAvailable || periodUs == 0) {
    return;
  }

  bool acceptPulse = false;

  // First few pulses establish the lock.
  if (tach.acceptedPulseCount < 5) {
    acceptPulse = true;
  } else {
    acceptPulse = isPeriodReasonable(periodUs, tach.lastGoodPeriodUs);
  }

  if (acceptPulse) {
    tach.lastGoodPeriodUs = periodUs;
    tach.acceptedPulseCount++;

    if (tach.acceptedPulseCount >= 5) {
      tach.locked = true;
    }

    tach.rpmRaw = (60.0 * 1000000.0) / (periodUs * PULSES_PER_REV);

    if (tach.rpmRaw <= MAX_VALID_RPM) {
      if (tach.rpmFiltered <= 0.0) {
        tach.rpmFiltered = tach.rpmRaw;
      } else {
        tach.rpmFiltered =
          (filterAlpha * tach.rpmRaw) + ((1.0 - filterAlpha) * tach.rpmFiltered);
      }
    }
  } else {
    tach.outlierRejectCount++;
  }
}

void resetTach1() {
  noInterrupts();
  s1_lastEdgeTimeUs = 0;
  s1_latestPeriodUs = 0;
  s1_newPulseAvailable = false;
  s1_pulseCount = 0;
  s1_fastRejectCount = 0;
  interrupts();

  tach1 = TachState();
}

void resetTach2() {
  noInterrupts();
  s2_lastEdgeTimeUs = 0;
  s2_latestPeriodUs = 0;
  s2_newPulseAvailable = false;
  s2_pulseCount = 0;
  s2_fastRejectCount = 0;
  interrupts();

  tach2 = TachState();
}

void updateOLED(
  float rpm1, float rpm2,
  unsigned long t1, unsigned long t2,
  int ir1, int ir2,
  bool lock1, bool lock2
) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Dual RPM Tach");

  display.setCursor(0, 8);
  display.print("S1 RPM: ");
  display.println(rpm1, 0);

  display.setCursor(0, 26);
  display.print("S2 RPM: ");
  display.println(rpm2, 0);

  display.setCursor(0, 40);
  display.print("T1:");
  display.print(t1);
  display.print(" T2:");
  display.println(t2);

  display.setCursor(0, 54);
  display.print("IR1:");
  display.print(ir1 == HIGH ? "H" : "L");
  display.print(" ");
  display.print(lock1 ? "Y" : "N");

  display.print(" IR2:");
  display.print(ir2 == HIGH ? "H" : "L");
  display.print(" ");
  display.print(lock2 ? "Y" : "N");

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);

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
  display.println("Dual tach starting...");
  display.display();

  delay(1000);

  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN), sensor1ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2_PIN), sensor2ISR, FALLING);

  Serial.println("==================================");
  Serial.println("Dual RPM Tachometer Started");
  Serial.println("Sensor 1: GPIO 27");
  Serial.println("Sensor 2: GPIO 26");
  Serial.println("Counting FALLING edge: HIGH -> LOW");
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

  // ---------- Copy Sensor 1 Interrupt Data ----------
  noInterrupts();
  bool s1PulseCopy = s1_newPulseAvailable;
  unsigned long s1PeriodCopy = s1_latestPeriodUs;
  unsigned long s1LastEdgeCopy = s1_lastEdgeTimeUs;
  unsigned long s1PulseCountCopy = s1_pulseCount;
  unsigned long s1FastRejectCopy = s1_fastRejectCount;
  s1_newPulseAvailable = false;
  interrupts();

  // ---------- Copy Sensor 2 Interrupt Data ----------
  noInterrupts();
  bool s2PulseCopy = s2_newPulseAvailable;
  unsigned long s2PeriodCopy = s2_latestPeriodUs;
  unsigned long s2LastEdgeCopy = s2_lastEdgeTimeUs;
  unsigned long s2PulseCountCopy = s2_pulseCount;
  unsigned long s2FastRejectCopy = s2_fastRejectCount;
  s2_newPulseAvailable = false;
  interrupts();

  // ---------- Process Pulses ----------
  processTachPulse(tach1, s1PulseCopy, s1PeriodCopy);
  processTachPulse(tach2, s2PulseCopy, s2PeriodCopy);

  // ---------- Timeout Sensor 1 ----------
  if (s1LastEdgeCopy > 0) {
    unsigned long s1TimeSincePulseMs = (nowUs - s1LastEdgeCopy) / 1000;

    if (s1TimeSincePulseMs > RPM_TIMEOUT_MS) {
      resetTach1();
      s1PeriodCopy = 0;
      s1PulseCountCopy = 0;
      s1FastRejectCopy = 0;
    }
  } else {
    tach1.rpmRaw = 0.0;
    tach1.rpmFiltered = 0.0;
  }

  // ---------- Timeout Sensor 2 ----------
  if (s2LastEdgeCopy > 0) {
    unsigned long s2TimeSincePulseMs = (nowUs - s2LastEdgeCopy) / 1000;

    if (s2TimeSincePulseMs > RPM_TIMEOUT_MS) {
      resetTach2();
      s2PeriodCopy = 0;
      s2PulseCountCopy = 0;
      s2FastRejectCopy = 0;
    }
  } else {
    tach2.rpmRaw = 0.0;
    tach2.rpmFiltered = 0.0;
  }

  int ir1State = digitalRead(SENSOR1_PIN);
  int ir2State = digitalRead(SENSOR2_PIN);

  // ---------- Serial Output ----------
  if (nowMs - lastSerialMs >= 250) {
    lastSerialMs = nowMs;

    Serial.print("S1 Raw: ");
    Serial.print(tach1.rpmRaw, 0);
    Serial.print(" | S1 Filt: ");
    Serial.print(tach1.rpmFiltered, 0);
    Serial.print(" | S1 T: ");
    Serial.print(s1PeriodCopy);
    Serial.print(" | S1 Lock: ");
    Serial.print(tach1.locked ? "Y" : "N");
    Serial.print(" | S1 A: ");
    Serial.print(tach1.acceptedPulseCount);
    Serial.print(" | S1 FastR: ");
    Serial.print(s1FastRejectCopy);
    Serial.print(" | S1 OutR: ");
    Serial.print(tach1.outlierRejectCount);

    Serial.print(" || ");

    Serial.print("S2 Raw: ");
    Serial.print(tach2.rpmRaw, 0);
    Serial.print(" | S2 Filt: ");
    Serial.print(tach2.rpmFiltered, 0);
    Serial.print(" | S2 T: ");
    Serial.print(s2PeriodCopy);
    Serial.print(" | S2 Lock: ");
    Serial.print(tach2.locked ? "Y" : "N");
    Serial.print(" | S2 A: ");
    Serial.print(tach2.acceptedPulseCount);
    Serial.print(" | S2 FastR: ");
    Serial.print(s2FastRejectCopy);
    Serial.print(" | S2 OutR: ");
    Serial.println(tach2.outlierRejectCount);
  }

  // ---------- OLED Output ----------
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    updateOLED(
      tach1.rpmRaw,
      tach2.rpmRaw,
      s1PeriodCopy,
      s2PeriodCopy,
      ir1State,
      ir2State,
      tach1.locked,
      tach2.locked
    );
  }
}