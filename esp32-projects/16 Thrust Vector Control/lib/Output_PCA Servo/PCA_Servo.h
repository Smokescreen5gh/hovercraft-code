#pragma once

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

class PCA_Servo
{
public:
    PCA_Servo(Adafruit_PWMServoDriver& pwmBoard,
              uint8_t channel,
              uint16_t minPulse = 120,
              uint16_t maxPulse = 500);

    void begin(int startAngle = 90);
    void write(int angle);

    int getAngle() const;

private:
    Adafruit_PWMServoDriver& _pwm;
    uint8_t _channel;

    uint16_t _minPulse;
    uint16_t _maxPulse;

    int _angle;
};