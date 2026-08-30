#include "DisplayManager.h"

// Constructor: store display settings and instance name
DisplayManager::DisplayManager(uint8_t screenWidth,
                               uint8_t screenHeight,
                               int8_t resetPin,
                               uint8_t i2cAddress,
                               TwoWire& wireBus,
                               uint8_t sdaPin,
                               uint8_t sclPin)
    : _screenWidth(screenWidth),
      _screenHeight(screenHeight),
      _resetPin(resetPin),
      _i2cAddress(i2cAddress),
      _sdaPin(sdaPin),
      _sclPin(sclPin),
      _wireBus(wireBus),
      _display(screenWidth, screenHeight, &wireBus, resetPin)
{
}

// METHOD 1: initialize OLED and I2C pins
bool DisplayManager::begin(const char*start_message)
{
    _wireBus.begin(_sdaPin, _sclPin);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, _i2cAddress))
    {
        return false;
    }

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println(start_message);
    _display.display();
    delay (1000);

    return true;
}

// METHOD 2: return display object reference
Adafruit_SSD1306& DisplayManager::getDisplay()
{
    return _display;
}

// METHOD 3: Display Any message
void DisplayManager::displayMessage(const char*message)
{
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println(message);
    _display.display();
    delay (100);
}

