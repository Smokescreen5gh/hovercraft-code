#include "Joystick.h"

// Constructor
Joystick::Joystick(uint8_t xPin,
                   uint8_t yPin,
                   uint8_t swPin,
                   const char* name,
                   ButtonMode buttonMode)
    : _xPin(xPin),
      _yPin(yPin),
      _swPin(swPin),
      _name(name),
      _x(0),
      _y(0),
      _xCenter(2048),
      _yCenter(2048),
      _deadzone(120),
      _buttonMode(buttonMode),
      _toggleState(false),
      _lastRawPressed(false),
      _buttonOutput(false),
      _pressedLevel(LOW)
{
}


// METHOD 1: Initialize joystick pins and deadzone
void Joystick::begin(int deadzone, PullMode pullMode)
{
    _deadzone = deadzone;

    pinMode(_xPin, INPUT);
    pinMode(_yPin, INPUT);

    if (pullMode == PullUp)
    {
        pinMode(_swPin, INPUT_PULLUP);
        _pressedLevel = LOW;
    }
    else if (pullMode == PullDown)
    {
        pinMode(_swPin, INPUT_PULLDOWN);
        _pressedLevel = HIGH;
    }
    else
    {
        pinMode(_swPin, INPUT);
    }
}


// METHOD 2: Calibrate joystick center
void Joystick::calibrate(int samples)
{
    long xSum = 0;
    long ySum = 0;

    for (int i = 0; i < samples; i++)
    {
        xSum += analogRead(_xPin);
        ySum += analogRead(_yPin);

        delay(3);
    }

    _xCenter = xSum / samples;
    _yCenter = ySum / samples;
}


// METHOD 3: Read joystick
void Joystick::update()
{
    _x = analogRead(_xPin);
    _y = analogRead(_yPin);

    readButton();
}


// Helper: Read joystick pushbutton
void Joystick::readButton()
{
    bool rawPressed = (digitalRead(_swPin) == _pressedLevel);

    if (_buttonMode == Momentary)
    {
        _buttonOutput = rawPressed;
    }
    else
    {
        if (!_lastRawPressed && rawPressed)
        {
            _toggleState = !_toggleState;
        }

        _buttonOutput = _toggleState;
    }

    _lastRawPressed = rawPressed;
}


// GETTERS
int Joystick::getX() const
{
    return _x;
}

int Joystick::getY() const
{
    return _y;
}

int Joystick::getXCenter() const
{
    return _xCenter;
}

int Joystick::getYCenter() const
{
    return _yCenter;
}

bool Joystick::getButtonState() const
{
    return _buttonOutput;
}

const char* Joystick::getName() const
{
    return _name;
}