#pragma once
#include <Arduino.h>

/*
============================================================
ShiftRegisterDriver.h

Low-level driver for daisy-chained shift registers.

Responsibilities:
- Controls DATA, CLOCK, LATCH pins
- Supports multiple chained registers
- Maintains internal output buffer
- Provides bit-level output control
- Pushes buffer to hardware on demand

Used by higher-level lighting modules (headlights, indicators, etc.)
Does NOT implement animations or lighting logic.
============================================================
*/

class ShiftRegisterDriver
{
public:

    // Constructor: define pins, number of registers, and instance name
    ShiftRegisterDriver(uint8_t dataPin,
                        uint8_t clockPin,
                        uint8_t latchPin,
                        uint8_t registerCount,
                        const char* name);

    // METHOD 1: initialize pins and allocate buffer
    void begin();
    
    // METHOD 2: clear entire software output buffer
    void clearAll();

    // METHOD 3: send buffer contents to shift registers
    void writeOutputs();

    // METHOD 4: set one output bit in buffer
    void setBit(uint16_t bitIndex, bool state);

    // METHOD 5: read one output bit from buffer
    bool getBit(uint16_t bitIndex) const;

    // METHOD 6: return total number of outputs
    uint16_t getTotalBits() const;

    // METHOD 7: return number of chained registers
    uint8_t getRegisterCount() const;

    // METHOD 8: return driver instance name
    const char* getName() const;

private:

    // Shift register control pins
    uint8_t _dataPin;
    uint8_t _clockPin;
    uint8_t _latchPin;

    // Number of daisy-chained registers
    uint8_t _registerCount;

    // Optional identifier string
    const char* _name;

    // Internal output buffer (1 byte per register)
    uint8_t* _buffer;

    // Helper: check if bit index is valid
    bool isValidBit(uint16_t bitIndex) const;
};