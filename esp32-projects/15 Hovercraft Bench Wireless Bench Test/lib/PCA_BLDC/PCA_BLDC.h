#pragma once

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

class PCA_BLDC
{
public:

    // Constructor: pass PCA board, channel, name, min throttle, max throttle
    PCA_BLDC(Adafruit_PWMServoDriver& pwmBoard,
             uint8_t channel,
             const char* name,
             uint16_t min_throttle,
             uint16_t max_throttle);

    // Motor state enum
    enum MotorState
    {
        bldc_ENABLED,
        bldc_STOPPED,
        bldc_DISABLED
    };

    // METHOD 1: Initializes the BLDC and its properties
    void begin();

    // METHOD 2: Allows the motor to run
    void enable();

    // METHOD 3: Disables the motor preventing it to run
    void disable();

    // METHOD 4: Stops the motor
    void stop();

    // METHOD 5: Sends the throttle PWM signal to the motor
    void setThrottle(int pwm_signal);

    // METHOD 6: Gives what state the motor is in
    char getState();

    // METHOD 7: Gives the ESC PWM value
    int getThrottle();

    // METHOD 8: Arms the BLDC motor
    void arm(int pwm_signal);

    const char* name;

private:

    Adafruit_PWMServoDriver& _pwm;
    uint8_t _channel;

    uint16_t _minThrottle;
    uint16_t _maxThrottle;

    int _currentThrottle;
    MotorState _state;

    // Helper: convert microseconds to PCA9685 pulse counts
    uint16_t microsecondsToPulse(uint16_t microseconds);
};