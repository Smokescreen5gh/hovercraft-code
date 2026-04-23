#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SENSOR_ADDR 0x28
#define OLED_ADDR   0x3C

#define SDA0 21
#define SCL0 22

#define SDA1 25
#define SCL1 26

TwoWire I2C_2 = TwoWire(1);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ELVH-L04D: ±4 inH2O differential, 10-90% transfer function
const float OFFSET_COUNTS     = 8192.0f;
const float FULL_SCALE_COUNTS = 13108.0f;
const float PRESSURE_RANGE_PA = 1990.6f;

float zeroOffset1 = 0.0f;
float zeroOffset2 = 0.0f;

bool readSensor(TwoWire &bus, uint16_t &counts, uint8_t &status)
{
    bus.requestFrom((uint8_t)SENSOR_ADDR, (uint8_t)4);

    if (bus.available() != 4) {
        return false;
    }

    uint8_t b1 = bus.read();
    uint8_t b2 = bus.read();
    bus.read(); // temp byte ignored
    bus.read(); // temp byte ignored

    status = (b1 >> 6) & 0x03;
    counts = ((uint16_t)(b1 & 0x3F) << 8) | b2;

    return true;
}

float countsToPressurePa(uint16_t counts, float offset)
{
    return ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA - offset;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA0, SCL0);
    I2C_2.begin(SDA1, SCL1);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("OLED failed");
        while (true) {
            delay(10);
        }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Static Pressure Tester");
    display.display();
    delay(2000);

    // --- zeroing ---
    Serial.println("Zeroing sensors, ensure no airflow...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Zeroing...");
    display.println("No airflow!");
    display.display();
    delay(2000);

    float acc1 = 0.0f, acc2 = 0.0f;
    int good1 = 0, good2 = 0;

    for (int i = 0; i < 32; i++) {
        uint16_t c; uint8_t s;
        if (readSensor(Wire,  c, s) && s == 0) {
            acc1 += ((float)c - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA;
            good1++;
        }
        if (readSensor(I2C_2, c, s) && s == 0) {
            acc2 += ((float)c - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA;
            good2++;
        }
        delay(20);
    }

    if (good1 > 0) {
        zeroOffset1 = acc1 / good1;
        Serial.print("S1 zero offset: ");
        Serial.print(zeroOffset1, 2);
        Serial.println(" Pa");
    } else {
        Serial.println("WARNING: S1 zeroing failed");
    }

    if (good2 > 0) {
        zeroOffset2 = acc2 / good2;
        Serial.print("S2 zero offset: ");
        Serial.print(zeroOffset2, 2);
        Serial.println(" Pa");
    } else {
        Serial.println("WARNING: S2 zeroing failed");
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Zeroing done!");
    display.display();
    delay(1000);
}

void loop()
{
    uint16_t counts1 = 0, counts2 = 0;
    uint8_t status1 = 0, status2 = 0;

    bool ok1 = readSensor(Wire,  counts1, status1);
    bool ok2 = readSensor(I2C_2, counts2, status2);

    float pressure1 = 0.0f;
    float pressure2 = 0.0f;

    if (ok1) pressure1 = countsToPressurePa(counts1, zeroOffset1);
    if (ok2) pressure2 = countsToPressurePa(counts2, zeroOffset2);

    Serial.print("S1: ");
    if (ok1) {
        Serial.print(pressure1, 1);
        Serial.print(" Pa  status=");
        Serial.print(status1);
    } else {
        Serial.print("READ FAIL");
    }

    Serial.print(" | S2: ");
    if (ok2) {
        Serial.print(pressure2, 1);
        Serial.print(" Pa  status=");
        Serial.print(status2);
    } else {
        Serial.print("READ FAIL");
    }
    Serial.println();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("Pressure Sensors");

    display.print("Static: ");
    if (ok1) {
        display.print(pressure1, 1);
        display.println(" Pa");
    } else {
        display.println("FAIL");
    }

    display.print("Venturi: ");
    if (ok2) {
        display.print(pressure2, 1);
        display.println(" Pa");
    } else {
        display.println("FAIL");
    }

    display.display();

    delay(300);
}