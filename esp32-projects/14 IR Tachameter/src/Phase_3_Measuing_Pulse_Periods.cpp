#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- Pulse Detection Variables ----------
int previousIRState = HIGH;
unsigned long pulseCount = 0;

// ---------- Timing Variables ----------
unsigned long lastPulseTimeUs = 0;
unsigned long periodUs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  previousIRState = digitalRead(IR_PIN);

  Serial.println("Phase 3: Manual Period Measurement Started");
  Serial.println("Detecting FALLING edge: HIGH -> LOW");
  Serial.println("Move tape past sensor multiple times.");
}

void loop() {
  int currentIRState = digitalRead(IR_PIN);

  // Detect falling edge: previous was HIGH, current is LOW
  if (previousIRState == HIGH && currentIRState == LOW) {
    pulseCount++;

    unsigned long nowUs = micros();

    if (lastPulseTimeUs == 0) {
      // First pulse only gives us a starting time.
      // We cannot calculate a period until the second pulse.
      Serial.println("First pulse detected. Waiting for next pulse...");
    } else {
      periodUs = nowUs - lastPulseTimeUs;

      Serial.print("Pulse detected! Count = ");
      Serial.print(pulseCount);

      Serial.print(" | Period = ");
      Serial.print(periodUs);
      Serial.println(" us");
    }

    // Save this pulse time so the next pulse can compare against it
    lastPulseTimeUs = nowUs;
  }

  // Save current state for the next loop
  previousIRState = currentIRState;
}