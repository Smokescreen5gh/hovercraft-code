//  TPIC6B595N how to use  
#include <Arduino.h>

// Physical wiring per your description + datasheet:
// GPIO 25 → IC pin 3  = SER IN  (serial data)
// GPIO 32 → IC pin 13 = SRCK    (shift register clock)
// GPIO 33 → IC pin 12 = RCK     (register/latch clock)

#define SER_DATA  25   // SER IN  — data
#define SER_CLK   32   // SRCK    — shift clock
#define SER_LATCH 33   // RCK     — latch

void writeRegister(uint8_t value)
{
  digitalWrite(SER_LATCH, LOW);
  shiftOut(SER_DATA, SER_CLK, MSBFIRST, value);
  digitalWrite(SER_LATCH, HIGH);
}

void setup()
{
  // Drive pins LOW BEFORE setting as OUTPUT
  // prevents a glitch during the pinMode call
  digitalWrite(SER_DATA,  LOW);
  digitalWrite(SER_CLK,   LOW);
  digitalWrite(SER_LATCH, LOW);

  pinMode(SER_DATA,  OUTPUT);
  pinMode(SER_CLK,   OUTPUT);
  pinMode(SER_LATCH, OUTPUT);

  // Datasheet section 7.3.2 says TI recommends clearing
  // the device during power up via SRCLR — but since you've
  // tied that pin to VCC (hardware), do it in software:
  // shift in all zeros = all transistors OFF = all LEDs OFF
  writeRegister(0x00);
}

void loop()
{
  writeRegister(0x00);
  delay (2000);

  writeRegister(0xFF);
  delay (2000);
}