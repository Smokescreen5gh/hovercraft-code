#include "PID_Display.h"

PID_Display::PID_Display(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void PID_Display::update(bool SystemEnabled,
                         float SetPoint,
                         float rpmFiltered,
                         float error,
                         int throttle,
                         float kp,
                         float ki,
                         float kd,
                         char state)
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(10, 0);
    display.println("Closed Loop Ctrl");

    // System state
    display.setCursor(35, 8);
    display.print(SystemEnabled ? "Enabled" : "Disabled");

    // Left column labels and fixed value positions
    display.setCursor(0, 20);
    display.print("SP:");
    display.setCursor(35, 20);
    display.print(SetPoint, 0);

    display.setCursor(0, 30);
    display.print("RPM:");
    display.setCursor(35, 30);
    display.print(rpmFiltered, 0);

    display.setCursor(0, 40);
    display.print("ERR:");
    display.setCursor(35, 40);
    display.print(error, 0);

    display.setCursor(0, 50);
    display.print("THR:");
    display.setCursor(35, 50);
    display.print(throttle);

    // Right column
    display.setCursor(72, 20);
    display.print("Kp:");
    display.setCursor(90, 20);
    display.print(kp, 3);

    display.setCursor(72, 30);
    display.print("Ki:");
    display.setCursor(90, 30);
    display.print(ki, 3);

    display.setCursor(72, 40);
    display.print("Kd:");
    display.setCursor(90, 40);
    display.print(kd, 3);

    display.setCursor(72, 50);
    display.print("M:");
    display.setCursor(90, 50);
    display.print(state);

    display.display();
}