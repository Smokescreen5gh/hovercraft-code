#include "ShiftRegisterDriver.h"

// Constructor: store pins, register count, and name
ShiftRegisterDriver::ShiftRegisterDriver(uint8_t dataPin,
                                         uint8_t clockPin,
                                         uint8_t latchPin,
                                         uint8_t registerCount,
                                         const char* name)
{
    _dataPin = dataPin;
    _clockPin = clockPin;
    _latchPin = latchPin;
    _registerCount = registerCount;
    _name = name;

    _buffer = nullptr;
}

// METHOD 1: initialize pins and allocate buffer
void ShiftRegisterDriver::begin()
{
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, LOW);
    digitalWrite(_latchPin, LOW);

    pinMode(_dataPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);

    _buffer = new uint8_t[_registerCount];

    clearAll();
    writeOutputs();
}

// METHOD 2: clear entire software output buffer
void ShiftRegisterDriver::clearAll()
{
    for (int i = 0; i < _registerCount; i++)
    {
        _buffer[i] = 0x00;
    }
}

// METHOD 3: send buffer contents to shift registers
void ShiftRegisterDriver::writeOutputs()
{
    digitalWrite(_latchPin, LOW);

    for (int i = _registerCount - 1; i >= 0; i--)
    {
        shiftOut(_dataPin, _clockPin, MSBFIRST, _buffer[i]);
    }

    digitalWrite(_latchPin, HIGH);
}

// METHOD 4: set one output bit in buffer
void ShiftRegisterDriver::setBit(uint16_t bitIndex, bool state)
{
    if (!isValidBit(bitIndex))
        return;

    uint8_t registerIndex = bitIndex / 8;
    uint8_t bitPosition = bitIndex % 8;

    if (state)
        _buffer[registerIndex] |= (1 << bitPosition);
    else
        _buffer[registerIndex] &= ~(1 << bitPosition);
}

// METHOD 5: read one output bit from buffer
bool ShiftRegisterDriver::getBit(uint16_t bitIndex) const
{
    if (!isValidBit(bitIndex))
        return false;

    uint8_t registerIndex = bitIndex / 8;
    uint8_t bitPosition = bitIndex % 8;

    return (_buffer[registerIndex] >> bitPosition) & 0x01;
}

// METHOD 6: return total number of outputs
uint16_t ShiftRegisterDriver::getTotalBits() const
{
    return _registerCount * 8;
}

// METHOD 7: return number of chained registers
uint8_t ShiftRegisterDriver::getRegisterCount() const
{
    return _registerCount;
}

// METHOD 8: return driver instance name
const char* ShiftRegisterDriver::getName() const
{
    return _name;
}

// Helper: check if bit index is valid
bool ShiftRegisterDriver::isValidBit(uint16_t bitIndex) const
{
    return (bitIndex < getTotalBits());
}