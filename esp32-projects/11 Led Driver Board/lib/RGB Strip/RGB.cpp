#include "RGB.h"

// Constructor: store strip pin, LED count, and instance name
RGBStrip::RGBStrip(uint8_t dataPin,
                   uint16_t ledCount,
                   const char* name)
{
    _dataPin = dataPin;
    _ledCount = ledCount;
    _name = name;

    _leds = nullptr;

    _isOn = false;
    _hue = 0;
    _brightness = 120;
}

// METHOD 1: initialize FastLED and clear strip
void RGBStrip::begin()
{
    _leds = new CRGB[_ledCount];

    FastLED.addLeds<WS2812, 26, GRB>(_leds, _ledCount);
    FastLED.clear();
    FastLED.show();

    _isOn = false;
}

// METHOD 2: turn strip ON using stored hue
void RGBStrip::turnOn()
{
    _isOn = true;
    showCurrentColor();
}

// METHOD 3: turn strip OFF but keep stored hue
void RGBStrip::turnOff()
{
    _isOn = false;

    for (uint16_t i = 0; i < _ledCount; i++)
    {
        _leds[i] = CRGB::Black;
    }

    FastLED.show();
}

// METHOD 4: set strip hue directly
void RGBStrip::setHue(uint8_t hue)
{
    _hue = hue;

    if (_isOn)
    {
        showCurrentColor();
    }
}

// METHOD 5: update hue from potentiometer value
void RGBStrip::updateFromPot(uint16_t potValue)
{
    uint8_t mappedHue = map(potValue, 0, 4095, 0, 255);
    setHue(mappedHue);
}

// METHOD 6: return true if strip is ON
bool RGBStrip::isOn() const
{
    return _isOn;
}

// METHOD 7: return current stored hue
uint8_t RGBStrip::getHue() const
{
    return _hue;
}

// METHOD 8: return strip instance name
const char* RGBStrip::getName() const
{
    return _name;
}

// Helper: apply current color state to strip
void RGBStrip::showCurrentColor()
{
    if (_leds == nullptr)
        return;

    for (uint16_t i = 0; i < _ledCount; i++)
    {
        _leds[i] = CHSV(_hue, 255, _brightness);
    }

    FastLED.show();
}