#pragma once
#include <Arduino.h>

typedef void (*AnimationFunc)(void (*writeRegister)(uint8_t));

struct AnimationEntry
{
    const char* name;
    AnimationFunc func;
};

void animSweepLeft(void (*writeRegister)(uint8_t));
void animSweepRight(void (*writeRegister)(uint8_t));
void animBlinkAll(void (*writeRegister)(uint8_t));
void animAlternating(void (*writeRegister)(uint8_t));
void animCenterOut(void (*writeRegister)(uint8_t));
void animOutsideIn(void (*writeRegister)(uint8_t));
void animFillBar(void (*writeRegister)(uint8_t));
void animEmptyBar(void (*writeRegister)(uint8_t));
void animFillSweepSix(void (*writeRegister)(uint8_t));

extern AnimationEntry animations[];
extern const int animationCount;