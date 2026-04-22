#include "Animations.h"

// ============================================================
// Helper state
//
// currentState stores the LED byte currently being shown.
// This lets us mimic the tutorial's regWrite(pin, state) behavior.
// ============================================================
static uint8_t currentState = 0x00;

// ============================================================
// ledMask
// Returns a mask for one LED bit
// ============================================================
static uint8_t ledMask(int index)
{
    return (1 << index);
}

// ============================================================
// clearAll
// Turns all LEDs off and resets local state
// ============================================================
static void clearAll(void (*writeRegister)(uint8_t))
{
    currentState = 0x00;
    writeRegister(currentState);
}

// ============================================================
// setLed
// Mimics the tutorial's regWrite(pin, state)
// ============================================================
static void setLed(void (*writeRegister)(uint8_t), int pin, bool state)
{
    if (state)
        currentState |= ledMask(pin);
    else
        currentState &= ~ledMask(pin);

    writeRegister(currentState);
}

// ============================================================
// effectA
//
// Tutorial behavior:
// for each i from left to right:
//   sweep a single LED from i to end
//   then leave LED i latched ON
// ============================================================
void effectA(void (*writeRegister)(uint8_t), int ledCount, int stepDelay)
{
    clearAll(writeRegister);

    for (int i = 0; i < ledCount; i++)
    {
        for (int k = i; k < ledCount; k++)
        {
            setLed(writeRegister, k, true);
            delay(stepDelay);
            setLed(writeRegister, k, false);
        }

        setLed(writeRegister, i, true);
    }
}

// ============================================================
// effectB
//
// Tutorial behavior:
// for each i from right to left:
//   sweep a single LED from left up to i-1
//   then leave LED i latched ON
// ============================================================
void effectB(void (*writeRegister)(uint8_t), int ledCount, int stepDelay)
{
    clearAll(writeRegister);

    for (int i = ledCount - 1; i >= 0; i--)
    {
        for (int k = 0; k < i; k++)
        {
            setLed(writeRegister, k, true);
            delay(stepDelay);
            setLed(writeRegister, k, false);
        }

        setLed(writeRegister, i, true);
    }
}

// ============================================================
// effectC
//
// Tutorial behavior:
// one lit LED moves left to right, then right to left
// ============================================================
void effectC(void (*writeRegister)(uint8_t), int ledCount, int stepDelay)
{
    clearAll(writeRegister);

    int prevI = 0;

    for (int i = 0; i < ledCount; i++)
    {
        setLed(writeRegister, prevI, false);
        setLed(writeRegister, i, true);
        prevI = i;
        delay(stepDelay);
    }

    for (int i = ledCount - 1; i >= 0; i--)
    {
        setLed(writeRegister, prevI, false);
        setLed(writeRegister, i, true);
        prevI = i;
        delay(stepDelay);
    }

    setLed(writeRegister, prevI, false);
}

// ============================================================
// effectD
//
// Tutorial behavior adapted to any ledCount:
// symmetric sweep from center outward, latching pairs on
//
// Best with even led counts like 6.
// For 6 LEDs, pairs are:
// (0,5), (1,4), (2,3)
// ============================================================
void effectD(void (*writeRegister)(uint8_t), int ledCount, int stepDelay)
{
    clearAll(writeRegister);

    int half = ledCount / 2;

    for (int i = 0; i < half; i++)
    {
        for (int k = i; k < half; k++)
        {
            setLed(writeRegister, k, true);
            setLed(writeRegister, (ledCount - 1) - k, true);
            delay(stepDelay);
            setLed(writeRegister, k, false);
            setLed(writeRegister, (ledCount - 1) - k, false);
        }

        setLed(writeRegister, i, true);
        setLed(writeRegister, (ledCount - 1) - i, true);
    }
}

// ============================================================
// effectE
//
// Tutorial behavior adapted to any ledCount:
// symmetric sweep from outer region inward, latching pairs on
// ============================================================
void effectE(void (*writeRegister)(uint8_t), int ledCount, int stepDelay)
{
    clearAll(writeRegister);

    int half = ledCount / 2;

    for (int i = half - 1; i >= 0; i--)
    {
        for (int k = 0; k <= i; k++)
        {
            setLed(writeRegister, k, true);
            setLed(writeRegister, (ledCount - 1) - k, true);
            delay(stepDelay);
            setLed(writeRegister, k, false);
            setLed(writeRegister, (ledCount - 1) - k, false);
        }

        setLed(writeRegister, i, true);
        setLed(writeRegister, (ledCount - 1) - i, true);
    }
}

// ============================================================
// Animation registry
// Names here are what will show on the OLED
// ============================================================
AnimationEntry animations[] =
{
    { "Effect A", effectA },
    { "Effect B", effectB },
    { "Effect C", effectC },
    { "Effect D", effectD },
    { "Effect E", effectE }
};

const int animationCount = sizeof(animations) / sizeof(animations[0]);