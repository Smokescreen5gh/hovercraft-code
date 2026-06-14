#include "TachDisplay.h"

TachDisplay::TachDisplay(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void TachDisplay::update(float rpmRaw,
                         float rpmFiltered,
                         unsigned long acceptedCount,
                         unsigned long outlierRejectCount,
                         unsigned long fastRejectCount)
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Hovercraft Tach");

    display.setCursor(0, 8);
    display.print("Raw: ");
    display.print(rpmRaw, 0);
    display.println(" RPM");

    display.setCursor(0, 24);
    display.print("Filt:");
    display.print(rpmFiltered, 0);
    display.println(" RPM");

    display.setCursor(0, 38);
    display.print("A:");
    display.print(acceptedCount);

    display.setCursor(64, 38);
    display.print("O:");
    display.print(outlierRejectCount);

    display.setCursor(0, 52);
    display.print("Fast:");
    display.print(fastRejectCount);

    display.display();
}