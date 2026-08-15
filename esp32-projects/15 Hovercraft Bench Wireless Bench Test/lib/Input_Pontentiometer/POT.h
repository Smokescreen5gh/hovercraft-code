// This file goes over the Potentiometer Class

#pragma once
#include <Arduino.h>

class Potentiometer {
public:
    // Constructor: pass pins, and name
    Potentiometer(uint8_t pin, const char* name);

    // METHOD 1: Initializes the potentiometer like pinmode and name
    void begin();

    // METHOD 2: Reads the current analog voltage from the potentiometer. 
    void update();

    // METHOD 3: Returns the most recently measuresd potentiometer value.
    uint16_t getValue();



private:
    uint8_t _pin;
    const char* _name;
    uint16_t _value;


};