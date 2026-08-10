#include "Transmitter_Info.h"

Transmitter_Info::Transmitter_Info(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void Transmitter_Info::update()
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(10, 0);
    display.println("HI");

    display.display();
}