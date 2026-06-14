#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class TachDisplay
{
public:
    TachDisplay(DisplayManager& displayManager);

    void update(float rpmRaw,
                float rpmFiltered,
                unsigned long acceptedCount,
                unsigned long outlierRejectCount,
                unsigned long fastRejectCount);

private:
    DisplayManager& _displayManager;
};