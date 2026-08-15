#pragma once
#include <Arduino.h>
#include <FastLED.h>

/*
============================================================
RGBStrip.h

Driver class for one addressable RGB strip.

Responsibilities:
- Owns FastLED strip configuration
- Controls strip ON/OFF state
- Stores current hue value
- Updates strip color from potentiometer input
- Restores last hue when turned back ON

Does NOT read the potentiometer directly.
Does NOT manage lighting system logic.
============================================================
*/

class RGBStrip
{
public:

    // Constructor: define strip pin, LED count, and instance name
    RGBStrip(uint8_t dataPin,
             uint16_t ledCount,
             const char* name);

    // METHOD 1: initialize FastLED and clear strip
    void begin();

    // METHOD 2: turn strip ON using stored hue
    void turnOn();

    // METHOD 3: turn strip OFF but keep stored hue
    void turnOff();

    // METHOD 4: set strip hue directly
    void setHue(uint8_t hue);

    // METHOD 5: update hue from potentiometer value
    void updateFromPot(uint16_t potValue);

    // METHOD 6: return true if strip is ON
    bool isOn() const;

    // METHOD 7: return current stored hue
    uint8_t getHue() const;

    // METHOD 8: return strip instance name
    const char* getName() const;

private:

    uint8_t _dataPin;
    uint16_t _ledCount;
    const char* _name;

    CRGB* _leds;

    bool _isOn;
    uint8_t _hue;
    uint8_t _brightness;

    // Helper: apply current color state to strip
    void showCurrentColor();
};