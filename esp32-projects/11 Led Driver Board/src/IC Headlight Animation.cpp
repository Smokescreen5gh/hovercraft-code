#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

// --------- Pin Definitions for OLED Screen ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// Create Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
//    0  Effect A
//    1  Effect B
//    2  Effect C
//    3  Effect D
//    4  Effect E
// ============================================================
const int  LED_COUNT          = 6;
const int  ANIM_SPEED         = 100;
const Mode currentMode        = MODE_CYCLE;
const int  specificAnimation  = 2;


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
// updateDisplay
// Shows the current animation name on the OLED.
// ============================================================
void updateDisplay(const char* animName)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("Headlights");
    display.println();

    display.print("Mode: ");
    if (currentMode == MODE_SPECIFIC)
        display.println("Specific");
    else
        display.println("Cycle");

    display.print("Anim: ");
    display.println(animName);

    display.print("LEDs: ");
    display.println(LED_COUNT);

    display.print("Speed: ");
    display.println(ANIM_SPEED);

    display.display();
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

    // Oled Screen Initialization
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
    {
        while (true) delay(1000);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Headlight Tester");
    display.println("Loading...");
    display.display();

    delay(2000);
}


// ============================================================
// loop
// ============================================================
void loop()
{
    switch (currentMode)
    {
        case MODE_SPECIFIC:
            updateDisplay(animations[specificAnimation].name);
            animations[specificAnimation].func(writeRegister, LED_COUNT, ANIM_SPEED);
            delay(500);
            break;

        case MODE_CYCLE:
            for (int i = 0; i < animationCount; i++)
            {
                updateDisplay(animations[i].name);
                animations[i].func(writeRegister, LED_COUNT, ANIM_SPEED);
                delay(500);
            }
            break;
    }
}