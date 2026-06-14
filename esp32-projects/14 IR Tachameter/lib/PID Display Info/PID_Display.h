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
                float kp,
                float ki,
                float kd,
                int throttle,
                char state);

private:
    DisplayManager& _displayManager;
};
