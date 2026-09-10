#include "PCA_Servo.h"

PCA_Servo::PCA_Servo(Adafruit_PWMServoDriver& pwmBoard,
                     uint8_t channel,
                     uint16_t minPulse,
                     uint16_t maxPulse)
    : _pwm(pwmBoard),
      _channel(channel),
      _minPulse(minPulse),
      _maxPulse(maxPulse),
      _angle(90)
{
}

void PCA_Servo::begin(int startAngle)
{
    write(startAngle);
}

void PCA_Servo::write(int angle)
{
    angle = constrain(angle, 0, 180);

    uint16_t pulse = map(angle,
                         0,
                         180,
                         _minPulse,
                         _maxPulse);

    _pwm.setPWM(_channel, 0, pulse);

    _angle = angle;
}

int PCA_Servo::getAngle() const
{
    return _angle;
}