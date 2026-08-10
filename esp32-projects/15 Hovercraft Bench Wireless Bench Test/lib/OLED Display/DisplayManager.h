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
                   TwoWire& wireBus,
                   uint8_t sdaPin,
                   uint8_t sclPin);

    // METHOD 1: initialize OLED and I2C pins
    bool begin(const char*start_message);

    // METHOD 2: return display object reference
    Adafruit_SSD1306& getDisplay();

    const char* getName() const;


private:
    uint8_t _screenWidth;
    uint8_t _screenHeight;
    int8_t _resetPin;
    uint8_t _i2cAddress;

    TwoWire& _wireBus;

    uint8_t _sdaPin;
    uint8_t _sclPin;
    

    Adafruit_SSD1306 _display;
};