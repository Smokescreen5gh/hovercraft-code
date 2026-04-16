#include "Animations.h"

static void allOff(void (*writeRegister)(uint8_t))
{
    writeRegister(0xFF);
}

void animSweepLeft(void (*writeRegister)(uint8_t))
{
    for (int i = 0; i < 6; i++)
    {
        uint8_t value = 0xFF;
        bitClear(value, i);
        writeRegister(value);
        delay(120);
    }
    allOff(writeRegister);
}

void animSweepRight(void (*writeRegister)(uint8_t))
{
    for (int i = 5; i >= 0; i--)
    {
        uint8_t value = 0xFF;
        bitClear(value, i);
        writeRegister(value);
        delay(120);
    }
    allOff(writeRegister);
}

void animBlinkAll(void (*writeRegister)(uint8_t))
{
    for (int i = 0; i < 4; i++)
    {
        writeRegister(0xC0); // LEDs 1-6 ON, 7-8 OFF
        delay(180);
        writeRegister(0xFF);
        delay(180);
    }
}

void animAlternating(void (*writeRegister)(uint8_t))
{
    for (int i = 0; i < 4; i++)
    {
        writeRegister(0b11101010); // 1,3,5 ON
        delay(180);
        writeRegister(0b11010101); // 2,4,6 ON
        delay(180);
    }
    writeRegister(0xFF);
}

void animCenterOut(void (*writeRegister)(uint8_t))
{
    writeRegister(0b11100111); // 3,4
    delay(180);

    writeRegister(0b11000011); // 2,3,4,5
    delay(180);

    writeRegister(0b11000000); // 1..6
    delay(250);

    writeRegister(0xFF);
}

void animOutsideIn(void (*writeRegister)(uint8_t))
{
    writeRegister(0b11011110); // 1,6
    delay(180);

    writeRegister(0b11011010); // 1,2,5,6
    delay(180);

    writeRegister(0b11000000); // 1..6
    delay(250);

    writeRegister(0xFF);
}

void animFillBar(void (*writeRegister)(uint8_t))
{
    uint8_t value = 0xFF;
    for (int i = 0; i < 6; i++)
    {
        bitClear(value, i);
        writeRegister(value);
        delay(150);
    }
    delay(250);
    writeRegister(0xFF);
}

void animEmptyBar(void (*writeRegister)(uint8_t))
{
    uint8_t value = 0b11000000; // 1..6 ON
    writeRegister(value);
    delay(250);

    for (int i = 5; i >= 0; i--)
    {
        bitSet(value, i);
        writeRegister(value);
        delay(150);
    }
    writeRegister(0xFF);
}

void animFillSweepSix(void (*writeRegister)(uint8_t))
{
    /*
    ============================================================
    LED DRIVER NOTE (TPIC6B595):
    0 = LED ON
    1 = LED OFF
    because this chip sinks current (active LOW outputs)
    ============================================================
    */

    /*
    ============================================================
    Frame table describing animation sequence

    Each entry represents the LED pattern at one moment in time.
    LEDs progressively move into place and stay ON.
    ============================================================
    */
    const uint8_t frames[] =
    {
        0b11000000, // - - - - - -
        0b11000001, // - - - - - +
        0b11000010, // - - - - + -
        0b11000100, // - - - + - -
        0b11001000, // - - + - - -
        0b11010000, // - + - - - -
        0b11100000, // + - - - - -
        0b11100001, // + - - - - +
        0b11100010, // + - - - + -
        0b11100100, // + - - + - -
        0b11101000, // + - + - - -
        0b11110000, // + + - - - -
        0b11110001, // + + - - - +
        0b11110010, // + + - - + -
        0b11110100, // + + - + - -
        0b11111000, // + + + - - -
        0b11111001, // + + + - - +
        0b11111010, // + + + - + -
        0b11111100, // + + + + - -
        0b11111101, // + + + + - +
        0b11111110, // + + + + + -
        0b11111111  // + + + + + +
    };

    /*
    ============================================================
    Useful constants
    ============================================================
    */
    const uint8_t ALL_OFF = 0b11000000;
    const uint8_t ALL_ON  = 0b11111111;
    const int speed = 50;
    const int brightness = 20;
    const int brighthigh = brightness * 10;
    const int brightlow = 1000 - brighthigh;

    const int frameCount = sizeof(frames) / sizeof(frames[0]);

    /*
    ============================================================
    STAGE 1
    Play animation frames at ~50% brightness

    Because TPIC6B595 has no hardware PWM,
    we simulate brightness using software PWM:

    ON time = OFF time → 50% brightness
    ============================================================
    */
    for (int i = 0; i < frameCount; i++)
    {
        for (int pwmCycle = 0; pwmCycle < speed; pwmCycle++)
        {
            writeRegister(frames[i]);   // show this animation frame
            delayMicroseconds(brighthigh);     // ON time

            writeRegister(ALL_OFF);     // temporarily turn LEDs off
            delayMicroseconds(brightlow);     // OFF time
        }
    }

    /*
    ============================================================
    STAGE 2
    After all LEDs reach final position,
    ramp brightness smoothly from 50% → 100%
    ============================================================
    */
    for (int duty = brightness; duty <= 100; duty += 3)
    {
        for (int pwmCycle = 0; pwmCycle < 80; pwmCycle++)
        {
            writeRegister(ALL_ON);                  // all LEDs ON
            delayMicroseconds(duty * 20);           // ON time increases

            writeRegister(ALL_OFF);                 // all LEDs OFF
            delayMicroseconds((100 - duty) * 20);  // OFF time decreases
        }
    }

    /*
    ============================================================
    STAGE 3
    Leave LEDs fully ON at maximum brightness
    ============================================================
    */
    writeRegister(ALL_ON);
}

AnimationEntry animations[] =
{
    {"Sweep Left",  animSweepLeft},
    {"Sweep Right", animSweepRight},
    {"Blink All",   animBlinkAll},
    {"Alternating", animAlternating},
    {"Center Out",  animCenterOut},
    {"Outside In",  animOutsideIn},
    {"Fill Bar",    animFillBar},
    {"Empty Bar",   animEmptyBar},
    {"Fill Sweep Six", animFillSweepSix},
};

const int animationCount = sizeof(animations) / sizeof(animations[0]);