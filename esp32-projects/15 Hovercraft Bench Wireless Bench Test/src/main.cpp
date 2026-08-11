#include <Arduino.h>
#include <ESP32Servo.h>

// Custom Libs
#include "DisplayManager.h"
#include "Transmitter_Info.h"

#include "bldc.h"
#include "Joystick.h"
#include "POT.h"
#include "Toggle.h"

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

// ------- Joy Stick Objects -----------
// Analog Joystick Pin Defintitions
Joystick joy1(32, 33, 25, "1", Joystick::Toggle);
Joystick joy2(26, 13, 4, "2", Joystick::Momentary);

// ------- Potentiometer Objects -----------
// Potentiometer Pin Definitions
#define POT1_PIN 36
#define POT2_PIN 39
#define POT3_PIN 34

Potentiometer pot1(POT1_PIN, "POT 1");
Potentiometer pot2(POT2_PIN, "POT 2");
Potentiometer pot3(POT3_PIN, "POT 3");

// ------- Toggle Switch Objects -----------
ToggleSwitch Switch1(27, "Light Switch");
ToggleSwitch Switch2(14, "Light Switch");
ToggleSwitch Switch3(35, "Light Switch");

void setup() {
  Serial.begin(115200);
  delay(200);

  // ================ Initialize the Elements ============
  oled_display.begin("Hovercraft Transmitter Mark 1");

  // Initialize Both Joystick modes
  joy1.begin(120, Joystick::PullUp);
  joy2.begin(120, Joystick::PullUp);

  // Potentiometer setup
  pot1.begin();
  pot2.begin();
  pot3.begin();

  // Intialize the Toggle Switches
  Switch1.begin();
  Switch2.begin();
  Switch3.begin();


  // ================== Calibration Message ================
  oled_display.displayMessage("Calibrating the Analog Sticks");
  delay (3000);

  // Calibrate the Joysticks
  joy1.calibrate();
  joy2.calibrate();



}

void loop() {
  static unsigned long lastDisplayMs = 0;
  unsigned long nowMs = millis();

  // -------------- Read Analog Joysticks ------------
  joy1.update();
  joy2.update();
  
  // ------------- Read Potentiometer Value ----------
  pot1.update();
  pot2.update();
  pot3.update();

  // ------------ Read ToggleSwitch Values ----------
  Switch1.update();
  Switch2.update();
  Switch3.update();


  // -------------- Display OLED -----------
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    transmitter_info.update(
      joy1.getX(),
      joy1.getY(),
      joy1.getButtonState(),

      joy2.getX(),
      joy2.getY(),
      joy2.getButtonState(),

      pot1.getValue(),
      pot2.getValue(),
      pot3.getValue(),

      Switch1.isOn(),
      Switch2.isOn(),
      Switch3.isOn()

    );
  }

  delay(50);

}
