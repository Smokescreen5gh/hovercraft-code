#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include "Animations.h"
#include "POT.h"

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

// --------- Pin Definitions for RGB Strip ----------
#define RGB_PIN 26
#define NUM_RGB 8
#define RGB_BRIGHTNESS 120

// --------- Potentiometer ----------
#define POT_PIN 27

// --------- Toggle Switch ----------
#define TOGGLE_PIN 14
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
CRGB leds[NUM_RGB];
Potentiometer rgbPot(POT_PIN, "RGB Pot");

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
//    5  Effect F
// ============================================================
const int  LED_COUNT          = 6;
const int  ANIM_SPEED         = 100;
const Mode currentMode        = MODE_SPECIFIC;
const int  specificAnimation  = 0;


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


void updateRGBfromPot()
{
    rgbPot.update();

    uint16_t potValue = rgbPot.getValue();          // 0 to 4095 on ESP32
    uint8_t hue = map(potValue, 0, 4095, 0, 255);  // map to HSV hue range

    fill_solid(leds, NUM_RGB, CHSV(hue, 255, RGB_BRIGHTNESS));
    FastLED.show();
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

    writeRegister(0x00); // Sets all LEDs to LOW


    // RGB Strip Initialization
    FastLED.addLeds<WS2812, RGB_PIN, GRB>(leds, NUM_RGB);
    FastLED.clear();
    FastLED.show();
    analogReadResolution(12);

    rgbPot.begin();

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
    updateRGBfromPot();

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
                updateRGBfromPot();
                updateDisplay(animations[i].name);
                animations[i].func(writeRegister, LED_COUNT, ANIM_SPEED);
                delay(500);
            }
            break;
    }
}

