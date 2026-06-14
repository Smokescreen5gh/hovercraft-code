#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1
#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

// ---------- Interrupt Variables ----------
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTimeUs = 0;
volatile unsigned long latestPeriodUs = 0;
volatile bool newPulseAvailable = false;
volatile unsigned long rejectedFastPulseCount = 0;

// This function runs automatically when GPIO 27 sees a falling edge
// This is the interupt function
void IRAM_ATTR handlePulse() {
  // Tracks what time of the clock the interrupt got triggered and stores it in the variable nowUs
  unsigned long nowUs = micros();

  // ---------------- Assign time to first pulse ----------------------
  // nowUs is stored in this variable and the period calcuation will start when the 2nd pulse and onwards commence
  if (lastPulseTimeUs == 0) {
    lastPulseTimeUs = nowUs;
    return;
  }

  
  //----------------- Calculate the period ---------------------------
  // After the second pulse, the period can be calculated
  unsigned long dt  = nowUs - lastPulseTimeUs;

  // ----------------- Reject Fake Pulse -----------------------------
  // Reject fake pulses that are too fast to be real
  // Checks if the period dt is fake or not
  if (dt < MIN_PULSE_INTERVAL_US) {
    rejectedFastPulseCount++;
    return;
  }

  // Stores the calculated period
  latestPeriodUs = dt;

  // The tracked pulse will now be considered as the prevoius pulse 
  lastPulseTimeUs = nowUs;
  

  // Increase Pulse Timer
  pulseCount++;
  newPulseAvailable = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(IR_PIN), handlePulse, FALLING);

  Serial.println("Phase 7: False Pulse Detection");
}

void loop() {
  bool pulseAvailableCopy;
  unsigned long periodCopy;
  unsigned long pulseCountCopy;

  noInterrupts();
  pulseAvailableCopy = newPulseAvailable;
  periodCopy = latestPeriodUs;
  pulseCountCopy = pulseCount;
  newPulseAvailable = false;
  interrupts();

  if (pulseAvailableCopy && periodCopy > 0) {
    float rpm = (60.0 * 1000000.0) / (periodCopy * PULSES_PER_REV);

    Serial.print("Pulse count: ");
    Serial.print(pulseCountCopy);

    Serial.print(" | Period us: ");
    Serial.print(periodCopy);

    Serial.print(" | RPM: ");
    Serial.println(rpm, 0);
    Serial.print(">rpm:");
    Serial.println(rpm);
  }
}