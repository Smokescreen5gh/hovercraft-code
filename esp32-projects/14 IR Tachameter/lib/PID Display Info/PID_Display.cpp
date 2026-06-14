#include "PID_Display.h"

PID_Display::PID_Display(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void PID_Display::update(bool SystemEnabled,
                        float SetPoint,
                        float rpmFiltered,
                        float error,
                        float kp,
                        float ki,
                        float kd,
                        int throttle,
                        char state)
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Closed Loop Control");

    display.setCursor(0, 8);
    display.print("RPM: ");
    display.print(rpmFiltered, 0);


    display.setCursor(0, 24);
    display.print("Setpoint:");
    display.print(SetPoint, 0);

    display.setCursor(0, 38);
    display.print("kp:");
    display.print(kp);

    display.setCursor(64, 38);
    display.print("ki:");
    display.print(ki);

    display.setCursor(0, 52);
    display.print("kd");
    display.print(kd);

    display.display();
}