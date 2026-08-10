#include <Arduino.h>
#include <ESP32Servo.h>

// Custom Libs
#include "DisplayManager.h"
#include "bldc.h"
#include "Transmitter_Info.h"

/*
 --------------------------------------------------------------
       Generate Objects
 --------------------------------------------------------------
*/

// ------- Display Object -----------
// OLED Pin Defintitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22
TwoWire I2C_0 = TwoWire(0);

DisplayManager oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_I2C_ADDRESS, I2C_0, SDA_PIN, SCL_PIN);
Transmitter_Info transmitter_info(oled_display);

void setup() {
  Serial.begin(115200);
  delay(200);

  oled_display.begin("Hovercraft Transmitter Mark 1");
}

void loop() {
  static unsigned long lastDisplayMs = 0;
  unsigned long nowMs = millis();

  // -------------- Display OLED -----------
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    transmitter_info.update();
  }

  delay(50);

}
