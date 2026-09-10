#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class PID_Display
{
public:
    PID_Display(DisplayManager& displayManager);

    //Method 1: gets the variabels and writes to various lines on the OLED Display 
    void update(bool SystemEnabled,
                    float SetPoint,
                    float rpmFiltered,
                    float error,
                    int throttle,
                    float kp,
                    float ki,
                    float kd,
                    char state);

private:
    DisplayManager& _displayManager;
};
