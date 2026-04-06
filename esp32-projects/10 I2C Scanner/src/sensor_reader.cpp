#include <Arduino.h>
#include <Wire.h>

static const uint8_t SENSOR_ADDR = 0x28;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  Serial.println("\nELVH raw read test");
}

void loop() {
  Wire.requestFrom(SENSOR_ADDR, (uint8_t)4);

  if (Wire.available() == 4) {
    uint8_t b1 = Wire.read();
    uint8_t b2 = Wire.read();
    uint8_t b3 = Wire.read();
    uint8_t b4 = Wire.read();

    uint8_t status = (b1 >> 6) & 0x03;
    uint16_t pressureCounts = ((uint16_t)(b1 & 0x3F) << 8) | b2;
    uint16_t tempCounts = ((uint16_t)b3 << 3) | (b4 >> 5);

    Serial.print("Bytes: ");
    Serial.print(b1); Serial.print(" ");
    Serial.print(b2); Serial.print(" ");
    Serial.print(b3); Serial.print(" ");
    Serial.println(b4);

    Serial.print("Status: ");
    Serial.print(status);
    Serial.print(" | Pressure Counts: ");
    Serial.print(pressureCounts);
    Serial.print(" | Temp Counts: ");
    Serial.println(tempCounts);
  } else {
    Serial.println("Read failed");
  }

  delay(1000);
}