#include "Headlight.h"

// Constructor: store shared driver, local LED count, offset, and name
HeadlightRect::HeadlightRect(ShiftRegisterDriver& driver,
                             uint8_t ledCount,
                             uint16_t offset,
                             const char* name)
    : _driver(driver)
{
    _ledCount = ledCount;
    _offset = offset;
    _name = name;

    _mode = OFF;
    _stepDelay = 100;
    _lastStepTime = 0;
    _currentStep = 0;
}

// METHOD 1: initialize rectangle state
void HeadlightRect::begin()
{
    turnOff();
}

// METHOD 2: update animation state without blocking
void HeadlightRect::update()
{
    if (_mode != STARTUP && _mode != SHUTDOWN)
        return;

    uint32_t now = millis();

    if (now - _lastStepTime < _stepDelay)
        return;

    _lastStepTime = now;

    uint8_t totalSteps = getStepCount();

    if (_mode == STARTUP)
    {
        if (_currentStep < totalSteps)
        {
            applyCenterFillStep(_currentStep, true);
            _driver.writeOutputs();
            _currentStep++;
        }
        else
        {
            _mode = ON;
        }
    }
    else if (_mode == SHUTDOWN)
    {
        if (_currentStep >= 0)
        {
            applyCenterFillStep(_currentStep, false);
            _driver.writeOutputs();
            _currentStep--;
        }
        else
        {
            _mode = OFF;
        }
    }
}

// METHOD 3: start center-fill startup animation
void HeadlightRect::startCenterFill(uint32_t stepDelay)
{
    _stepDelay = stepDelay;
    _lastStepTime = millis();
    _currentStep = 0;

    clearLocalRange();
    _driver.writeOutputs();

    _mode = STARTUP;
}

// METHOD 4: start reverse center-fill shutdown animation
void HeadlightRect::startShutdown(uint32_t stepDelay)
{
    _stepDelay = stepDelay;
    _lastStepTime = millis();
    _currentStep = getStepCount() - 1;

    _mode = SHUTDOWN;
}

// METHOD 5: turn all local LEDs fully OFF
void HeadlightRect::turnOff()
{
    clearLocalRange();
    _driver.writeOutputs();
    _mode = OFF;
}

// METHOD 6: turn all local LEDs fully ON
void HeadlightRect::turnFullyOn()
{
    for (uint8_t i = 0; i < _ledCount; i++)
    {
        setLocalLED(i, true);
    }

    _driver.writeOutputs();
    _mode = ON;
}

// METHOD 7: return true if rectangle is fully OFF
bool HeadlightRect::isOff() const
{
    return (_mode == OFF);
}

// METHOD 8: return true if rectangle is fully ON
bool HeadlightRect::isOn() const
{
    return (_mode == ON);
}

// METHOD 9: return true if rectangle is animating
bool HeadlightRect::isAnimating() const
{
    return (_mode == STARTUP || _mode == SHUTDOWN);
}

// METHOD 10: return rectangle instance name
const char* HeadlightRect::getName() const
{
    return _name;
}

// Helper: set one local LED by local index
void HeadlightRect::setLocalLED(uint8_t localIndex, bool state)
{
    if (localIndex >= _ledCount)
        return;

    _driver.setBit(_offset + localIndex, state);
}

// Helper: clear only this rectangle's LED range
void HeadlightRect::clearLocalRange()
{
    for (uint8_t i = 0; i < _ledCount; i++)
    {
        setLocalLED(i, false);
    }
}

// Helper: apply one center-fill step
void HeadlightRect::applyCenterFillStep(uint8_t step, bool state)
{
    if (_ledCount == 0)
        return;

    // Odd LED count (ex: 7): center first, then pairs outward
    if (_ledCount % 2 == 1)
    {
        int center = _ledCount / 2;

        if (step == 0)
        {
            setLocalLED(center, state);
            return;
        }

        int offset = step;
        int left = center - offset;
        int right = center + offset;

        if (left >= 0)
            setLocalLED(left, state);

        if (right < _ledCount)
            setLocalLED(right, state);
    }
    // Even LED count: middle pair first, then pairs outward
    else
    {
        int leftCenter = (_ledCount / 2) - 1;
        int rightCenter = _ledCount / 2;

        int left = leftCenter - step;
        int right = rightCenter + step;

        if (left >= 0)
            setLocalLED(left, state);

        if (right < _ledCount)
            setLocalLED(right, state);
    }
}

// Helper: return number of center-fill steps
uint8_t HeadlightRect::getStepCount() const
{
    if (_ledCount == 0)
        return 0;

    if (_ledCount % 2 == 1)
        return (_ledCount / 2) + 1;   // ex: 7 LEDs -> 4 steps

    return (_ledCount / 2);           // ex: 6 LEDs -> 3 steps
}