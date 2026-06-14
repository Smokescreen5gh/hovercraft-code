#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// ---------- Pulse Detection Variables ----------
int previousIRState = HIGH;
unsigned long pulseCount = 0;

// ---------- Timing Variables ----------
unsigned long lastPulseTimeUs = 0;
unsigned long periodUs = 0;

// ---------- RPM Variable ----------
float rpm = 0.0;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  previousIRState = digitalRead(IR_PIN);

  Serial.println("Phase 4: Manual RPM Calculation Started");
  Serial.println("Detecting FALLING edge: HIGH -> LOW");
}

void loop() {
  int currentIRState = digitalRead(IR_PIN);

  // Detect falling edge: previous was HIGH, current is LOW
  if (previousIRState == HIGH && currentIRState == LOW) {
    pulseCount++;

    unsigned long nowUs = micros();

    if (lastPulseTimeUs == 0) {
      Serial.println("First pulse detected. Waiting for next pulse...");
    } else {
      periodUs = nowUs - lastPulseTimeUs;

      rpm = (60.0 * 1000000.0) / (periodUs * PULSES_PER_REV);

      Serial.print("Pulse count = ");
      Serial.print(pulseCount);

      Serial.print(" | Period = ");
      Serial.print(periodUs);
      Serial.print(" us");

      Serial.print(" | RPM = ");
      Serial.println(rpm, 0);
    }

    // Save this pulse time for the next period calculation
    lastPulseTimeUs = nowUs;
  }

  previousIRState = currentIRState;
}