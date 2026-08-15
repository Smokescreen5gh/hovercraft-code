#pragma once
#include <Arduino.h>

// ============================================================
// Mode selection
//
// MODE_SPECIFIC -> run one chosen animation repeatedly
// MODE_CYCLE    -> run every animation in order
// ============================================================
enum Mode
{
    MODE_SPECIFIC,
    MODE_CYCLE,
};

// ============================================================
// Animation function type
//
// writeRegister : sends one byte to the shift register
// ledCount      : number of LEDs being controlled
// stepDelay     : animation speed in ms
// ============================================================
typedef void (*AnimationFunc)(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);

// ============================================================
// Animation table entry
//
// name : animation name for OLED display
// func : pointer to animation function
// ============================================================
struct AnimationEntry
{
    const char* name;
    AnimationFunc func;
};

// ============================================================
// Tutorial-based effect declarations
// These are adapted from the original 74HC595 example
// ============================================================
void effectA(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);
void effectB(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);
void effectC(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);
void effectD(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);
void effectE(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);
void effectF(void (*writeRegister)(uint8_t), int ledCount, int stepDelay);

// ============================================================
// Animation registry
// ============================================================
extern AnimationEntry animations[];
extern const int animationCount;