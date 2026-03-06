// This file implements the BLDC Class Method

#include "bldc.h"

// Define the Constructor
// ---------------------- Constructor ----------------------
BLDC::BLDC(uint8_t pin, const char* name, uint16_t min_throttle, uint16_t max_throttle)
    :
    _pin(pin),
    name(name),
    _minThrottle(min_throttle),
    _maxThrottle(max_throttle),
    _currentThrottle(min_throttle),
    _state(BLDC::bldc_DISABLED)
{

}

// ------------------- METHOD 1 Begin() ----------------------
void BLDC::begin() {

    esc.setPeriodHertz(50);                         // ESC Standard Frequency
    esc.attach(_pin, _minThrottle, _maxThrottle);   // Attach the ESC to the Pin

    esc.writeMicroseconds(_minThrottle);            // Sets the esc to the lowest possible
    _currentThrottle = _minThrottle;                // Updates the currentThrottle variable
}

// ------------------- METHOD 2 Enable() ----------------------
void BLDC::enable() {
    _state = BLDC::bldc_DISABLED;
}

// ------------------- METHOD 3 Disable() ----------------------
void BLDC::disable() {
        esc.writeMicroseconds(_minThrottle);   // Immediately stop the motor
    _currentThrottle = _minThrottle;       // Update stored throttle
    _state = BLDC::bldc_DISABLED;         // Lock the motor
}

// ------------------- METHOD 4 Stop() ----------------------
void BLDC::stop() {
    esc.writeMicroseconds(_minThrottle);   // Stop motor but keep it armed
    _currentThrottle = _minThrottle;       // Update stored throttle
    _state = BLDC::bldc_STOPPED;
}

// ------------------- METHOD 5 setThrottle() ----------------------
void BLDC::setThrottle(int pwm_signal) { //pwm signal from pot is parsed into this

    // Disabled State (Ignore PWM Signal)
    if (_state == BLDC::bldc_DISABLED) {
        return;
    }

    // Clamp Throttle values within safe range
        // If signal is less than the minimum Throttle specified
        if (pwm_signal < _minThrottle) {
            pwm_signal = _minThrottle;
        }

        // If signal is more than the maximum Throttle specified
        if (pwm_signal > _maxThrottle) {
            pwm_signal = _maxThrottle;
        }

    // Send PWM Signal to the ESC
    esc.writeMicroseconds(pwm_signal);

    // Update stored throttle value
    _currentThrottle = pwm_signal;

    // Update Motor State
    if (pwm_signal == _minThrottle) {

        _state = BLDC::bldc_STOPPED; // Sets motor to stop condition when it recieves the minimum pwm value

    } else {
        _state = BLDC::bldc_ENABLED; // Sets motor to enable condition when it recieves some signal greater than the minimum pwm value
    }
}

// ------------------- METHOD 6 getState() ----------------------
char BLDC::getState() {
    if (_state == bldc_ENABLED) {
        return 'E';
    }
    else if (_state == bldc_DISABLED) {
        return 'D';
    } 
    else if (_state == bldc_STOPPED) {
        return 'S';
    } 

    return 'D';
}

// ------------------- METHOD 7 getThrottle() ----------------------
int BLDC::getThrottle() {
    return _currentThrottle;
}

// ------------------- METHOD 8 arm() ----------------------
void BLDC::arm(int pwm_signal) {
    if (pwm_signal == _minThrottle) {
        _state = bldc_ENABLED;
    }

}