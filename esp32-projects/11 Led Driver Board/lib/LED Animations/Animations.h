#pragma once
#include <Arduino.h>

// ============================================================
// AnimationFunc
// Every animation takes three arguments:
//   writeRegister  — function that sends a byte to the IC
//   ledCount       — how many LEDs are wired up
//   speed          — delay in ms controlling how fast it runs
// ============================================================
typedef void (*AnimationFunc)(void (*writeRegister)(uint8_t), int ledCount, int speed);

// ============================================================
// AnimationEntry
// Bundles an animation name and function together as one unit
// ============================================================
struct AnimationEntry
{
    const char*   name;  // e.g. "Chase"
    AnimationFunc func;
};

// ============================================================
// Mode — controls what main.cpp does in loop()
// ============================================================
enum Mode
{
    MODE_SPECIFIC,  // play one chosen animation on repeat
    MODE_CYCLE      // play every animation in order
};

// ============================================================
// Animation declarations
// ============================================================
void animChase        (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animChaseReverse (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animPingPong     (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animScanner      (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animBlinkAll     (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animAlternating  (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animFillBar      (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animEmptyBar     (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animCenterOut    (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animOutsideIn    (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animCometFill    (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animCometFillRev (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animDualFillIn   (void (*writeRegister)(uint8_t), int ledCount, int speed);
void animDualFillOut  (void (*writeRegister)(uint8_t), int ledCount, int speed);

// ============================================================
// Animation table — defined in Animations.cpp
// ============================================================
extern AnimationEntry animations[];
extern const int      animationCount;
