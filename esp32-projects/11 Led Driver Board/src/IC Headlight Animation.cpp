#include <Arduino.h>
#include "Animations.h"

// ============================================================
// Pin definitions
// GPIO 25 → IC pin 3  = SER IN  (data)
// GPIO 32 → IC pin 13 = SRCK    (shift clock)
// GPIO 33 → IC pin 12 = RCK     (latch)
// ============================================================
#define SER_DATA  25
#define SER_CLK   32
#define SER_LATCH 33

// ============================================================
// SETTINGS — the only lines you ever change
//
// LED_COUNT    how many LEDs you have wired up
// ANIM_SPEED   delay in ms between steps — lower = faster
//
// currentMode:
//   MODE_SPECIFIC  →  one animation on repeat
//   MODE_CYCLE     →  every animation in order
//
// specificAnimation:
//   index into animations[]
//    0  Chase
//    1  Chase Reverse
//    2  Ping Pong
//    3  Scanner
//    4  Blink All
//    5  Alternating
//    6  Fill Bar
//    7  Empty Bar
//    8  Center Out
//    9  Outside In
//   10  Comet Fill
//   11  Comet Fill Rev
//   12  Dual Fill In
//   13  Dual Fill Out
// ============================================================
const int  LED_COUNT          = 6;
const int  ANIM_SPEED         = 80;
const Mode currentMode        = MODE_CYCLE;
const int  specificAnimation  = 0;


// ============================================================
// writeRegister
// Sends one byte to the shift register.
// Passed into every animation as a function pointer.
// ============================================================
void writeRegister(uint8_t value)
{
    digitalWrite(SER_LATCH, LOW);
    shiftOut(SER_DATA, SER_CLK, MSBFIRST, value);
    digitalWrite(SER_LATCH, HIGH);
}


// ============================================================
// setup
// ============================================================
void setup()
{
    digitalWrite(SER_DATA,  LOW);
    digitalWrite(SER_CLK,   LOW);
    digitalWrite(SER_LATCH, LOW);

    pinMode(SER_DATA,  OUTPUT);
    pinMode(SER_CLK,   OUTPUT);
    pinMode(SER_LATCH, OUTPUT);

    writeRegister(0x00);
}


// ============================================================
// loop
// ============================================================
void loop()
{
    switch (currentMode)
    {
        case MODE_SPECIFIC:
            animations[specificAnimation].func(writeRegister, LED_COUNT, ANIM_SPEED);
            delay(500);
            break;

        case MODE_CYCLE:
            for (int i = 0; i < animationCount; i++)
            {
                animations[i].func(writeRegister, LED_COUNT, ANIM_SPEED);
                delay(500);
            }
            break;
    }
}
