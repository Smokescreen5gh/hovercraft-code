#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "bldc.h"
#include "FanTester.h"

// OLED + Static sensor bus
#define SDA0 21
#define SCL0 22

// Venturi sensor second I2C bus
#define SDA1 25
#define SCL1 26

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define MOTOR_PIN 4
#define POT_PIN 34

TwoWire I2C_2 = TwoWire(1);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BLDC motor(MOTOR_PIN, "M1", 1000, 2000);
FanTester fanTester(Wire, I2C_2);

static uint16_t potRaw = 0;
static uint16_t throttleUs = 1000;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting Benchtop Flow Test...");

  Wire.begin(SDA0, SCL0);
  I2C_2.begin(SDA1, SCL1);

  pinMode(POT_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Benchtop Flow Test");
  display.println("Zeroing sensors...");
  display.display();

  motor.begin();
  fanTester.begin();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Zeroing done");
  display.display();
  delay(1000);
}

void loop() {
  potRaw = analogRead(POT_PIN);
  throttleUs = map(potRaw, 0, 4095, 1000, 2000);

  motor.arm(throttleUs);
  motor.setThrottle(throttleUs);

  static uint32_t lastSensorMs = 0;
  static uint32_t dataPointCounter = 0;  // <- ADD THIS

  if (millis() - lastSensorMs >= 50) {
    lastSensorMs = millis();
    fanTester.update();
  }

  static uint32_t lastSerialMs = 0;
  if (millis() - lastSerialMs >= 250) {
    lastSerialMs = millis();
    dataPointCounter++;  // <- INCREMENT EACH OUTPUT

    Serial.print("Time: ");
    Serial.print(millis());
    Serial.print(" ms | ");
    
    Serial.print("Static: ");
    Serial.print(fanTester.getStaticPaFiltered(), 2);
    Serial.print(" Pa | ");
    
    Serial.print("Venturi: ");
    Serial.print(fanTester.getVenturiPaFiltered(), 2);
    Serial.print(" Pa | ");
    
    Serial.print("Flow: ");
    Serial.print(fanTester.getFlowM3s(), 4);
    Serial.print(" m³/s | ");
    Serial.println();
    
    /*Serial.print("Throttle: ");
    Serial.println(motor.getThrottle());
    Serial.print(millis());
    Serial.print(",");
    Serial.print(fanTester.getStaticPressurePa(), 2);
    Serial.print(",");
    Serial.print(fanTester.getStaticPaFiltered(), 2);
    Serial.print(",");
    Serial.print(fanTester.getVenturiPressurePa(), 2);
    Serial.print(",");
    Serial.print(fanTester.getVenturiPaFiltered(), 2);
    Serial.print(",");
    Serial.print(fanTester.getFlowM3s(), 4);
    Serial.print(",");
    Serial.print(motor.getThrottle());
    Serial.println(); */

    /*Serial.print("Pot: ");       Serial.print(potRaw);
    Serial.print(" | PWM: ");    Serial.print(throttleUs);
    Serial.print(" | Static: "); Serial.print(fanTester.getStaticPressurePa(), 1); Serial.print(" Pa");
    Serial.print(" | dP: ");     Serial.print(fanTester.getVenturiPressurePa(), 1); Serial.print(" Pa");
    Serial.print(" | V1: ");     Serial.print(fanTester.getV1(), 2);
    Serial.print(" | V2: ");     Serial.print(fanTester.getV2(), 2);
    Serial.print(" | Q: ");      Serial.print(fanTester.getFlowM3s(), 4);
    Serial.print(" | Motor: ");  Serial.println(motor.getState()); */
  }

  static uint32_t lastOledMs = 0;
  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Benchtop Flow");
    display.print(dataPointCounter);  // <- DISPLAY COUNTER

    display.setCursor(0, 10);
    display.print("PWM:");
    display.print(motor.getThrottle());
    display.print(" M:");
    display.print(motor.getState());

    display.setCursor(0, 22);
    display.print("S:");
    display.print(fanTester.getStaticPaFiltered(), 0);
    display.print(" dP:");
    display.print(fanTester.getVenturiPaFiltered(), 0);

    display.setCursor(0, 34);
    display.print("V1:");
    display.print(fanTester.getV1(), 1);
    display.print(" V2:");
    display.print(fanTester.getV2(), 1);

    display.setCursor(0, 46);
    display.print("Q:");
    display.print(fanTester.getFlowM3s(), 4);

    display.setCursor(0, 56);
    display.print("S:");
    display.print(fanTester.staticSensorOK() ? "Y" : "N");
    display.print(" V:");
    display.print(fanTester.venturiSensorOK() ? "Y" : "N");

    display.display();
  }
}