#include <Arduino.h>
#include <Wire.h>

static const uint8_t SENSOR_ADDR = 0x28;

// L04D: ±4 inH2O differential, 10-90% transfer function
const float OFFSET_COUNTS      = 8192.0f;
const float FULL_SCALE_COUNTS  = 13108.0f;
const float PRESSURE_RANGE_PA  = 1990.6f;

bool readSensor(uint16_t &counts, uint8_t &status)
{
    Wire.requestFrom(SENSOR_ADDR, (uint8_t)4);

    if (Wire.available() != 4) return false;

    uint8_t b1 = Wire.read();
    uint8_t b2 = Wire.read();
    Wire.read(); // temp, ignored for now
    Wire.read(); // temp, ignored for now

    status = (b1 >> 6) & 0x03;
    counts = ((uint16_t)(b1 & 0x3F) << 8) | b2;

    return true;
}

float countsToPressurePa(uint16_t counts)
{
    return ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);
    Serial.println("\nELVH debug test");
    Serial.println("counts | delta_from_8192 | pressure_Pa | status");
}

void loop()
{
    uint16_t counts = 0;
    uint8_t  status = 0;

    if (readSensor(counts, status)) {
        float pressure = countsToPressurePa(counts);

        Serial.print("counts=");
        Serial.print(counts);
        Serial.print(" | delta=");
        Serial.print((int)counts - 8192);
        Serial.print(" | pressure=");
        Serial.print(pressure, 2);
        Serial.print(" Pa | status=");
        Serial.println(status);
    } else {
        Serial.println("READ FAIL - check wiring");
    }

    delay(500);
}