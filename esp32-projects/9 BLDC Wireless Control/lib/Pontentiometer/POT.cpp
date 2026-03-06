// This file implements the Potentiometer Class Method

#include "POT.h"

// Define the Constructor
// ---------------------- Constructor ----------------------
Potentiometer::Potentiometer(uint8_t pin, const char* name)
    :
        _pin(pin),
        _name(name),
        _value(0)

    {

    }

// ------------------- METHOD 1 Begin() ----------------------
void Potentiometer::begin() {

    pinMode(_pin, INPUT);
    _value = analogRead(_pin);

}

// ------------------- METHOD 2 Update() ----------------------
void Potentiometer::update() {
    _value = analogRead(_pin);
}

// ------------------- METHOD 3 getValue() ----------------------
uint16_t Potentiometer::getValue() {
    return _value;
}