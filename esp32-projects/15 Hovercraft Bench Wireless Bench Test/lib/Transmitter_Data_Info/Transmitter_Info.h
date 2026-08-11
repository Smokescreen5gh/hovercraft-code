#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class Transmitter_Info
{
public:
    Transmitter_Info(DisplayManager& DisplayManager);

    //Method 1: Get the variables (input variables, incoming telemetry) and writes to various lines on the OLED Display
    void update(
                const char* RADIO_TX_ADDR,
                const char* RADIO_RX_ADDR,
                int joy1x, 
                int joy1y,
                bool joy1button,
                int joy2x,
                int joy2y,
                bool joy2button,
                uint16_t pot1,
                uint16_t pot2,
                uint16_t pot3,
                bool switch1,
                bool switch2,
                bool switch3,
                bool connected,
                int randomRx );

private:
    DisplayManager& _displayManager;
};