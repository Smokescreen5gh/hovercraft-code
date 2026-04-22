#include <Arduino.h>
#include "ShiftRegisterDriver.h"
#include "Headlight.h"
#include "Toggle.h"

// ============================================================
// Pin definitions
// ============================================================
#define SER_DATA   25
#define SER_CLK    32
#define SER_LATCH  33
#define TOGGLE_PIN 14

// ============================================================
// Driver instances
// ============================================================
ShiftRegisterDriver headlightDriver(SER_DATA, SER_CLK, SER_LATCH, 1, "Headlight Registers");

// One 7-LED rectangle using outputs 0 through 6
HeadlightRect rect1(headlightDriver, 6, 0, "Rect 1");

// Toggle switch on GPIO 14
ToggleSwitch lightSwitch(TOGGLE_PIN, "Light Switch");

// ============================================================
// setup
// ============================================================
void setup()
{
    headlightDriver.begin();
    rect1.begin();
    lightSwitch.begin();
}

// ============================================================
// loop
// ============================================================
void loop()
{
    lightSwitch.update();

    if (lightSwitch.isOn())
    {
        if (rect1.isOff())
        {
            rect1.startCenterFill(120);
        }
    }
    else
    {
        if (rect1.isOn())
        {
            rect1.startShutdown(120);
        }
    }

    rect1.update();
}