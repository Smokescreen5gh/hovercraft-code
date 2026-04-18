#include <Arduino.h>

#define DATA_PIN 25
#define CLOCK_PIN 32
#define LATCH_PIN 33

void writeRegister(uint8_t value)
{
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
  digitalWrite(LATCH_PIN, HIGH);
}

void setup()
{
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  writeRegister(0xFF);
}

void loop()
{
  for (int i = 0; i < 6; i++)
  {
    uint8_t value = 0xFF;
    bitClear(value, i);   // active LOW = LED ON
    writeRegister(value);
    delay(150);
  }

  writeRegister(0xFF);
  delay(400);
}