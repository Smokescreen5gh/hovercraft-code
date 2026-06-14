// --------- Include the Libraries ----------
#include <Arduino.h>


// --------- IR Sensor ----------
#define IR_PIN 27


void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);


  // Serial Monitor Text
  Serial.println("TCRT5000 IR Sensor Test Started");
  Serial.println("Rotate the impeller slowly by hand.");
}

void loop() {
  // Reads the IR Sensor
  int irState = digitalRead(IR_PIN);

  // Prints To Serial Monitor
  Serial.print("IR D0 State: ");
  Serial.println(irState);

  delay(100);
}