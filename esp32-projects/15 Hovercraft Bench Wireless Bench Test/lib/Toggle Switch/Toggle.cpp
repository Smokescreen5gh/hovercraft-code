#include "Toggle.h"

ToggleSwitch::ToggleSwitch(uint8_t pin, const char* name)
{
    _pin = pin;
    _name = name;
}

void ToggleSwitch::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    update();
}

void ToggleSwitch::update()
{
    _status = (digitalRead(_pin) == LOW);
}

bool ToggleSwitch::isOn() const
{
    return _status;
}