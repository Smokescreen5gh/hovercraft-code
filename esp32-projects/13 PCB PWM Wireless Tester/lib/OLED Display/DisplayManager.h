#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
============================================================
DisplayManager.h

Low-level driver for the OLED display.

Responsibilities:
- Initializes I2C and SSD1306 display
- Owns the display object
- Exposes display access to higher-level code

Does NOT decide what text to print.
Screen content is handled in main.cpp or system logic.
============================================================
*/

class DisplayManager
{
public:

    // Constructor: define screen settings and instance name
    DisplayManager(uint8_t screenWidth,
                   uint8_t screenHeight,
                   int8_t resetPin,
                   uint8_t i2cAddress,
                   const char* name);

    // METHOD 1: initialize OLED and I2C pins
    void begin(uint8_t sdaPin, uint8_t sclPin);

    // METHOD 2: return display object reference
    Adafruit_SSD1306& getDisplay();

    // METHOD 3: return display instance name
    const char* getName() const;

private:
    uint8_t _screenWidth;
    uint8_t _screenHeight;
    int8_t _resetPin;
    uint8_t _i2cAddress;
    const char* _name;

    Adafruit_SSD1306 _display;
};