#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// ---------- Interrupt Variables ----------
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTimeUs = 0;
volatile unsigned long latestPeriodUs = 0;
volatile bool newPulseAvailable = false;

// This function runs automatically when GPIO 27 sees a falling edge
// This is the interupt function
void IRAM_ATTR handlePulse() {
  // Tracks what time of the clock the interrupt got triggered and stores it in the variable nowUs
  unsigned long nowUs = micros();

  // For the first pulse only
  // nowUs is stored in this variable and the period calcuation will start when the 2nd pulse and onwards commence
  if (lastPulseTimeUs == 0) {
    lastPulseTimeUs = nowUs;
    return;
  }

  // After the second pulse, the period can be calculated
  // Calculate the period 
  latestPeriodUs = nowUs - lastPulseTimeUs;

  // The tracked pulse will now be considered as the prevoius pulse 
  lastPulseTimeUs = nowUs;

  pulseCount++;
  newPulseAvailable = true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(IR_PIN), handlePulse, FALLING);

  Serial.println("Phase 6B: Interrupt Period Measurement Started");
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