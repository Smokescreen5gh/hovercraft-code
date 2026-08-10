#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class Transmitter_Info
{
public:
    Transmitter_Info(DisplayManager& DisplayManager);

    //Method 1: Get the variables (input variables, incoming telemetry) and writes to various lines on the OLED Display
    void update();

private:
    DisplayManager& _displayManager;
};