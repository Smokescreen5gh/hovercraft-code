#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- Pulse Detection Variables ----------
int previousIRState = HIGH;
unsigned long pulseCount = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  // Read the starting state of the sensor
  previousIRState = digitalRead(IR_PIN);

  Serial.println("Phase 2: Manual Pulse Detection Started");
  Serial.println("Move reflective tape in front of the sensor.");
  Serial.println("Looking for FALLING edge: HIGH -> LOW");
}

void loop() {
  int currentIRState = digitalRead(IR_PIN);

  // Detect falling edge: previous was HIGH, current is LOW
  if (previousIRState == HIGH && currentIRState == LOW) {
    pulseCount++;

    Serial.print("Pulse detected! Count = ");
    Serial.println(pulseCount);
  }

  // Save current state for the next loop
  previousIRState = currentIRState;
}