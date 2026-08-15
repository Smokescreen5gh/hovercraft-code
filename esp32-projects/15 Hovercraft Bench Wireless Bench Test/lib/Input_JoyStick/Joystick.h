#pragma once
#include <Arduino.h>

class Joystick
{
public:
    enum ButtonMode
    {
        Momentary,
        Toggle
    };

    enum PullMode
    {
        PullUp,
        PullDown,
        NoPull
    };

    Joystick(uint8_t xPin,
             uint8_t yPin,
             uint8_t swPin,
             const char* name,
             ButtonMode buttonMode);

    // Initialize joystick pins and deadzone
    void begin(int deadzone = 120, PullMode pullMode = PullUp);

    // Find joystick center
    void calibrate(int samples = 200);

    // Read X, Y, and button
    void update();

    // Getters
    int getX() const;
    int getY() const;

    int getXCenter() const;
    int getYCenter() const;

    bool getButtonState() const;

    const char* getName() const;

private:
    // Variables lock in the xpin, ypin, and switch pin, and name
    uint8_t _xPin;
    uint8_t _yPin;
    uint8_t _swPin;

    const char* _name;

    // Variables lock in the x and y analogread and the pressed state
    int _x;
    int _y;

    int _xCenter;
    int _yCenter;

    int _deadzone;

    //Button Attributes
    ButtonMode _buttonMode;
    bool _toggleState;
    bool _lastRawPressed;
    bool _buttonOutput;

    int _pressedLevel;

    void readButton();
};