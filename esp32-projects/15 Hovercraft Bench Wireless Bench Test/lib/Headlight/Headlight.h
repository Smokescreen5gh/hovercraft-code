#pragma once
#include <Arduino.h>
#include "ShiftRegisterDriver.h"

/*
============================================================
HeadlightRect.h

Driver class for one rectangular headlight section.

Responsibilities:
- Controls a local LED range inside a shared shift register chain
- Runs non-blocking startup and shutdown animations
- Supports fully ON and fully OFF states
- Uses center-fill startup and reverse center-fill shutdown

Does NOT own shift register hardware directly.
Uses ShiftRegisterDriver to control shared outputs.
============================================================
*/

class HeadlightRect
{
public:

    // Constructor: define shared driver, local LED count, offset, and name
    HeadlightRect(ShiftRegisterDriver& driver,
                  uint8_t ledCount,
                  uint16_t offset,
                  const char* name);

    // METHOD 1: initialize rectangle state
    void begin();

    // METHOD 2: update animation state without blocking
    void update();

    // METHOD 3: start center-fill startup animation
    void startCenterFill(uint32_t stepDelay);

    // METHOD 4: start reverse center-fill shutdown animation
    void startShutdown(uint32_t stepDelay);

    // METHOD 5: turn all local LEDs fully OFF
    void turnOff();

    // METHOD 6: turn all local LEDs fully ON
    void turnFullyOn();

    // METHOD 7: return true if rectangle is fully OFF
    bool isOff() const;

    // METHOD 8: return true if rectangle is fully ON
    bool isOn() const;

    // METHOD 9: return true if rectangle is animating
    bool isAnimating() const;

    // METHOD 10: return rectangle instance name
    const char* getName() const;

private:

    // Rectangle operating state
    enum Mode
    {
        OFF,
        STARTUP,
        ON,
        SHUTDOWN
    };

    // Shared shift register driver
    ShiftRegisterDriver& _driver;

    // Local rectangle configuration
    uint8_t _ledCount;
    uint16_t _offset;
    const char* _name;

    // Animation timing and progress
    Mode _mode;
    uint32_t _stepDelay;
    uint32_t _lastStepTime;
    int _currentStep;

    // Helper: set one local LED by local index
    void setLocalLED(uint8_t localIndex, bool state);

    // Helper: clear only this rectangle's LED range
    void clearLocalRange();

    // Helper: apply one center-fill step
    void applyCenterFillStep(uint8_t step, bool state);

    uint8_t getStepCount() const;
};