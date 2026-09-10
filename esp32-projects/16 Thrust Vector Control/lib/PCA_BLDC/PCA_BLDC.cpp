#include "PCA_BLDC.h"

// ---------------------- Constructor ----------------------
PCA_BLDC::PCA_BLDC(Adafruit_PWMServoDriver& pwmBoard,
                   uint8_t channel,
                   const char* name,
                   uint16_t min_throttle,
                   uint16_t max_throttle)
    :
    _pwm(pwmBoard),
    _channel(channel),
    name(name),
    _minThrottle(min_throttle),
    _maxThrottle(max_throttle),
    _currentThrottle(min_throttle),
    _state(PCA_BLDC::bldc_DISABLED)
{
}


// ------------------- METHOD 1 Begin() ----------------------
void PCA_BLDC::begin()
{
    uint16_t pulse = microsecondsToPulse(_minThrottle);

    _pwm.setPWM(_channel, 0, pulse);

    _currentThrottle = _minThrottle;
}


// ------------------- METHOD 2 Enable() ----------------------
void PCA_BLDC::enable()
{
    _state = PCA_BLDC::bldc_ENABLED;
}


// ------------------- METHOD 3 Disable() ----------------------
void PCA_BLDC::disable()
{
    uint16_t pulse = microsecondsToPulse(_minThrottle);

    _pwm.setPWM(_channel, 0, pulse);

    _currentThrottle = _minThrottle;
    _state = PCA_BLDC::bldc_DISABLED;
}


// ------------------- METHOD 4 Stop() ----------------------
void PCA_BLDC::stop()
{
    uint16_t pulse = microsecondsToPulse(_minThrottle);

    _pwm.setPWM(_channel, 0, pulse);

    _currentThrottle = _minThrottle;
    _state = PCA_BLDC::bldc_STOPPED;
}


// ------------------- METHOD 5 setThrottle() ----------------------
void PCA_BLDC::setThrottle(int pwm_signal)
{
    // Disabled State
    if (_state == PCA_BLDC::bldc_DISABLED)
    {
        return;
    }

    // Clamp throttle
    if (pwm_signal < _minThrottle)
    {
        pwm_signal = _minThrottle;
    }

    if (pwm_signal > _maxThrottle)
    {
        pwm_signal = _maxThrottle;
    }

    // Convert microseconds to PCA9685 counts
    uint16_t pulse = microsecondsToPulse(pwm_signal);

    // Send signal to ESC
    _pwm.setPWM(_channel, 0, pulse);

    _currentThrottle = pwm_signal;

    // Update motor state
    if (pwm_signal == _minThrottle)
    {
        _state = PCA_BLDC::bldc_STOPPED;
    }
    else
    {
        _state = PCA_BLDC::bldc_ENABLED;
    }
}


// ------------------- METHOD 6 getState() ----------------------
char PCA_BLDC::getState()
{
    if (_state == bldc_ENABLED)
    {
        return 'E';
    }
    else if (_state == bldc_DISABLED)
    {
        return 'D';
    }
    else if (_state == bldc_STOPPED)
    {
        return 'S';
    }

    return 'D';
}


// ------------------- METHOD 7 getThrottle() ----------------------
int PCA_BLDC::getThrottle()
{
    return _currentThrottle;
}


// ------------------- METHOD 8 arm() ----------------------
void PCA_BLDC::arm(int pwm_signal)
{
    if (pwm_signal == _minThrottle)
    {
        _state = bldc_ENABLED;
    }
}


// ------------------- Helper ----------------------
uint16_t PCA_BLDC::microsecondsToPulse(uint16_t microseconds)
{
    // PCA9685 is running at 50 Hz
    // One period = 20,000 microseconds
    // PCA9685 has 4096 counts per period

    return ((uint32_t)microseconds * 4096) / 20000;
}