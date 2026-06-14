// This file goes over the Brushless Motor Class

#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class BLDC {
public:

    // Constructor: pass pins, name
    BLDC(uint8_t pin, const char* name, uint16_t min_throttle, uint16_t max_throttle);

    // Motor state enum
    enum MotorState {
        bldc_ENABLED,
        bldc_STOPPED,
        bldc_DISABLED
    };


    // METHOD 1: Initializes the BLDC and its properties (pin, hertz)
    void begin();

    // METHOD 2: Allows the motor to run
    void enable();

    // METHOD 3: Disables the motor preventing it to run
    void disable();

    // METHOD 4: Stops the motor 
    void stop();

    // METHOD 5: Sends the throttle pwm signal to the motor
    void setThrottle(int pwm_signal);

    // METHOD 6: Gives what state the motor is in Enable, Disable, Stop
    char getState();

    // Method 7: Gives the ESC PWM value the ESC is getting
    int getThrottle();

    // METHOD 8: Arms the BLDC Motor
    void arm(int pwm_signal);

    const char* name;

private:

    Servo esc;               // Servo object controlling the ESC
    uint8_t _pin;            // ESC signal pin

    uint16_t _minThrottle;        // Minimum throttle pulse
    uint16_t _maxThrottle;        // Maximum throttle pulse

    int _currentThrottle;    // Last throttle value sent
    MotorState _state;       // Current motor state

};