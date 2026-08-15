#pragma once

#include <Arduino.h>
#include "DisplayManager.h"

class Reciever_Info
{
public:
    Reciever_Info(DisplayManager& DisplayManager);

    //Method 1: Get the variables (input variables, incoming telemetry) and writes to various lines on the OLED Display
    void update(
                const char* RADIO_TX_ADDR,
                const char* RADIO_RX_ADDR,
                int Servo_1, 
                int Servo_2, 
                int Servo_3,
                int Servo_4,
                uint16_t pot1,
                bool connected);

private:
    DisplayManager& _displayManager;
};