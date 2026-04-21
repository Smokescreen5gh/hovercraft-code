#include "Animations.h"

// ============================================================
// SINK MODE REFERENCE
//
//   ledOn  → clears the bit → transistor OFF → LED ON
//   ledOff → sets the bit   → transistor ON  → LED OFF
//
//   ALL_OFF = 0xFF         every LED off
//   ALL_ON  = 0b11000000   LEDs 0-5 on, unused pins 6-7 off
//
// Note: ALL_ON only works for exactly 6 LEDs on bits 0-5.
// For animations that build state incrementally, we use
// ALL_OFF as the starting point and ledOn() each LED we want.
// ============================================================

#define ALL_OFF 0xFF
#define ALL_ON  0b11000000

static inline void ledOn (uint8_t &state, int led) { bitClear(state, led); }
static inline void ledOff(uint8_t &state, int led) { bitSet  (state, led); }

static inline void show(void (*writeRegister)(uint8_t), uint8_t state)
{
    writeRegister(state);
}

// ============================================================
// allOn
// Builds an ALL_ON state for any ledCount, not just 6.
// Sets bits 0 through ledCount-1 to 0 (LED on).
// Bits above ledCount stay 1 (LED off / unused).
// ============================================================
static uint8_t buildAllOn(int ledCount)
{
    uint8_t state = ALL_OFF;
    for (int i = 0; i < ledCount; i++)
    {
        ledOn(state, i);
    }
    return state;
}


// ============================================================
// animChase
// One LED travels left to right, two passes.
// ============================================================
void animChase(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    for (int pass = 0; pass < 2; pass++)
    {
        for (int i = 0; i < ledCount; i++)
        {
            uint8_t state = ALL_OFF;
            ledOn(state, i);
            show(writeRegister, state);
            delay(speed);
        }
    }

    show(writeRegister, ALL_OFF);
}


// ============================================================
// animChaseReverse
// One LED travels right to left, two passes.
// ============================================================
void animChaseReverse(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    for (int pass = 0; pass < 2; pass++)
    {
        for (int i = ledCount - 1; i >= 0; i--)
        {
            uint8_t state = ALL_OFF;
            ledOn(state, i);
            show(writeRegister, state);
            delay(speed);
        }
    }

    show(writeRegister, ALL_OFF);
}


// ============================================================
// animPingPong
// One LED bounces back and forth 
// Keeps previous LED off and only lights the current one.
// ============================================================
void animPingPong(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    int prevI = 0;
    uint8_t state = ALL_OFF;

    // forward pass  0 → ledCount-1
    for (int i = 0; i < ledCount; i++)
    {
        ledOff(state, prevI);
        ledOn (state, i);
        show(writeRegister, state);
        prevI = i;
        delay(speed);
    }

    // reverse pass  ledCount-1 → 0
    for (int i = ledCount - 1; i >= 0; i--)
    {
        ledOff(state, prevI);
        ledOn (state, i);
        show(writeRegister, state);
        prevI = i;
        delay(speed);
    }

    show(writeRegister, ALL_OFF);
}


// ============================================================
// animScanner
// Knight rider: moving LED with a dimmer trail behind it.
// Trail uses software PWM — flickered on briefly each cycle.
// speed controls ms per position step.
// ============================================================
void animScanner(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    const int trailDuty = 25;   // trail brightness 0-100%
    int pos = 0;
    int dir = 1;

    int totalSteps = ledCount * 2 * 2;  // 2 full bounces

    for (int step = 0; step < totalSteps; step++)
    {
        int trail = pos - dir;

        unsigned long start = millis();
        while (millis() - start < (unsigned long)speed)
        {
            uint8_t state = ALL_OFF;
            ledOn(state, pos);

            if (trail >= 0 && trail < ledCount)
            {
                show(writeRegister, state);
                delayMicroseconds(trailDuty * 100);

                ledOn(state, trail);
                show(writeRegister, state);
                delayMicroseconds((100 - trailDuty) * 100);
            }
            else
            {
                show(writeRegister, state);
                delayMicroseconds(1000);
            }
        }

        pos += dir;
        if (pos >= ledCount - 1) dir = -1;
        if (pos <= 0)            dir =  1;
    }

    show(writeRegister, ALL_OFF);
}


// ============================================================
// animBlinkAll
// All LEDs blink together 5 times.
// speed controls on/off duration.
// ============================================================
void animBlinkAll(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t allOn = buildAllOn(ledCount);

    for (int i = 0; i < 5; i++)
    {
        show(writeRegister, allOn);
        delay(speed);
        show(writeRegister, ALL_OFF);
        delay(speed);
    }
}


// ============================================================
// animAlternating
// Even LEDs (0,2,4) and odd LEDs (1,3,5) swap back and forth.
// speed controls how long each state holds.
// ============================================================
void animAlternating(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t even = ALL_OFF;
    uint8_t odd  = ALL_OFF;

    for (int i = 0; i < ledCount; i++)
    {
        if (i % 2 == 0) ledOn(even, i);
        else            ledOn(odd,  i);
    }

    for (int i = 0; i < 6; i++)
    {
        show(writeRegister, even);
        delay(speed);
        show(writeRegister, odd);
        delay(speed);
    }

    show(writeRegister, ALL_OFF);
}


// ============================================================
// animFillBar
// LEDs fill up one by one left to right, then clear.
// speed controls delay between each LED lighting.
// ============================================================
void animFillBar(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;

    for (int i = 0; i < ledCount; i++)
    {
        ledOn(state, i);            // add one more, keep previous ones
        show(writeRegister, state);
        delay(speed);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animEmptyBar
// All LEDs start on, turn off one by one right to left.
// speed controls delay between each LED going off.
// ============================================================
void animEmptyBar(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = buildAllOn(ledCount);

    show(writeRegister, state);
    delay(speed * 3);

    for (int i = ledCount - 1; i >= 0; i--)
    {
        ledOff(state, i);
        show(writeRegister, state);
        delay(speed);
    }
}


// ============================================================
// animCenterOut
// LEDs expand outward from the center.
// Works for any even ledCount.
// ============================================================
void animCenterOut(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;
    int mid = ledCount / 2;

    for (int i = 0; i < mid; i++)
    {
        ledOn(state, mid - 1 - i);   // left of center
        ledOn(state, mid + i);       // right of center
        show(writeRegister, state);
        delay(speed);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animOutsideIn
// LEDs collapse inward from the outer edges.
// Works for any even ledCount.
// ============================================================
void animOutsideIn(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;
    int mid = ledCount / 2;

    for (int i = 0; i < mid; i++)
    {
        ledOn(state, i);                  // from left edge
        ledOn(state, ledCount - 1 - i);   // from right edge
        show(writeRegister, state);
        delay(speed);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animCometFill
// GitHub effectA converted.
//
// For each LED i, a dot runs from i to the end then stays.
// Looks like a comet landing and staying in place.
// Fills up left to right.
// ============================================================
void animCometFill(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;

    for (int i = 0; i < ledCount; i++)
    {
        // comet runs from i forward to the end
        for (int k = i; k < ledCount; k++)
        {
            ledOn(state, k);            // light the comet dot
            show(writeRegister, state);
            delay(speed);
            ledOff(state, k);           // turn it off again
        }

        // LED i has now "arrived" — leave it on permanently
        ledOn(state, i);
        show(writeRegister, state);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animCometFillRev
// GitHub effectB converted.
//
// Same as animCometFill but fills right to left.
// ============================================================
void animCometFillRev(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;

    for (int i = ledCount - 1; i >= 0; i--)
    {
        // comet runs from i backward to 0
        for (int k = i; k >= 0; k--)
        {
            ledOn(state, k);
            show(writeRegister, state);
            delay(speed);
            ledOff(state, k);
        }

        // LED i has arrived — leave it on permanently
        ledOn(state, i);
        show(writeRegister, state);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animDualFillIn
// GitHub effectD converted.
//
// Two comets start at opposite ends and move toward the center.
// Each comet permanently lights its LED when it arrives.
// Fills from outside in.
// ============================================================
void animDualFillIn(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;
    int half = ledCount / 2;

    for (int i = 0; i < half; i++)
    {
        // comets run from i inward toward center
        for (int k = i; k < half; k++)
        {
            ledOn(state, k);                  // left comet
            ledOn(state, ledCount - 1 - k);   // right comet (mirror)
            show(writeRegister, state);
            delay(speed);
            ledOff(state, k);
            ledOff(state, ledCount - 1 - k);
        }

        // both LEDs at position i have arrived — leave them on
        ledOn(state, i);
        ledOn(state, ledCount - 1 - i);
        show(writeRegister, state);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// animDualFillOut
// GitHub effectE converted.
//
// Two comets start at the center and move outward.
// Each comet permanently lights its LED when it arrives.
// Fills from inside out.
// ============================================================
void animDualFillOut(void (*writeRegister)(uint8_t), int ledCount, int speed)
{
    uint8_t state = ALL_OFF;
    int half = ledCount / 2;

    for (int i = half - 1; i >= 0; i--)
    {
        // comets run from center outward to i
        for (int k = half - 1; k >= i; k--)
        {
            ledOn(state, k);
            ledOn(state, ledCount - 1 - k);
            show(writeRegister, state);
            delay(speed);
            ledOff(state, k);
            ledOff(state, ledCount - 1 - k);
        }

        // both LEDs at position i have arrived — leave them on
        ledOn(state, i);
        ledOn(state, ledCount - 1 - i);
        show(writeRegister, state);
    }

    delay(speed * 3);
    show(writeRegister, ALL_OFF);
}


// ============================================================
// Animation table
//
// To add a new animation:
//   1. Write the function above
//   2. Declare it in Animations.h
//   3. Add one row here
//   main.cpp never changes
// ============================================================
AnimationEntry animations[] =
{
    {"Chase",           animChase},
    {"Chase Reverse",   animChaseReverse},
    {"Ping Pong",       animPingPong},
    {"Scanner",         animScanner},
    {"Blink All",       animBlinkAll},
    {"Alternating",     animAlternating},
    {"Fill Bar",        animFillBar},
    {"Empty Bar",       animEmptyBar},
    {"Center Out",      animCenterOut},
    {"Outside In",      animOutsideIn},
    {"Comet Fill",      animCometFill},
    {"Comet Fill Rev",  animCometFillRev},
    {"Dual Fill In",    animDualFillIn},
    {"Dual Fill Out",   animDualFillOut},
};

const int animationCount = sizeof(animations) / sizeof(animations[0]);
