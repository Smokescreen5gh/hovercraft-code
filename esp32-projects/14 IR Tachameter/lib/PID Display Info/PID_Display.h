#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class PID_Display
{
public:
    PID_Display(DisplayManager& displayManager);

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
