#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class Step4_Display
{
public:
    Step4_Display(DisplayManager& DisplayManager);

    //Method 1: Get the variables (input variables, incoming telemetry) and writes to various lines on the OLED Display
    void update(
                int angle1,
                int angle2);

private:
    DisplayManager& _displayManager;
};