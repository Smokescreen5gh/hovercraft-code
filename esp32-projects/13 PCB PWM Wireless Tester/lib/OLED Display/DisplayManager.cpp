#include "DisplayManager.h"

// Constructor: store display settings and instance name
DisplayManager::DisplayManager(uint8_t screenWidth,
                               uint8_t screenHeight,
                               int8_t resetPin,
                               uint8_t i2cAddress,
                               const char* name)
    : _display(screenWidth, screenHeight, &Wire, resetPin)
{
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _resetPin = resetPin;
    _i2cAddress = i2cAddress;
    _name = name;
}

// METHOD 1: initialize OLED and I2C pins
void DisplayManager::begin(uint8_t sdaPin, uint8_t sclPin)
{
    Wire.begin(sdaPin, sclPin);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, _i2cAddress))
    {
        while (true)
        {
            delay(1000);
        }
    }

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.display();
}

// METHOD 2: return display object reference
Adafruit_SSD1306& DisplayManager::getDisplay()
{
    return _display;
}

// METHOD 3: return display instance name
const char* DisplayManager::getName() const
{
    return _name;
}