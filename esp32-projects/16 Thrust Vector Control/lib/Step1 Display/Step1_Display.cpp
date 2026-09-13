#include "Step1_Display.h"


Step1_Display::Step1_Display(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void Step1_Display::update(
                int angle1,
                int angle2)
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Servo 1
    display.setCursor(0, 0);
    display.print("Servo 1 Angle: ");

    display.setCursor(85, 0);
    display.print(angle1);

    // Servo 2
    display.setCursor(0, 10);
    display.print("Servo 2 Angle: ");

    display.setCursor(85, 8);
    display.print(angle2);


    display.display();
}