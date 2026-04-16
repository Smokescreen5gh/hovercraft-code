#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include "Animations.h"

#define DATA_PIN   25
#define CLOCK_PIN  32
#define LATCH_PIN  33

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define SCREEN_ADDR  0x3C

// RGB strip
#define RGB_PIN   26
#define NUM_LEDS  10   // change this later
CRGB leds[NUM_LEDS];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void writeRegister(uint8_t value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

void showAnimationName(const char* name)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Running:");

    display.setTextSize(2);
    display.setCursor(0, 16);
    display.println(name);

    display.display();
}

void setup()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    writeRegister(0xFF); // all off

    Wire.begin(21, 22);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR))
    {
        while (true) { }
    }

    // RGB strip init
    FastLED.addLeds<WS2812B, RGB_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.setBrightness(80);
    FastLED.show();

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 16);
    display.println("LED Demo");
    display.display();
    delay(1200);
}

void loop()
{
    static bool ranOnce = false;

    if (!ranOnce)
    {
        showAnimationName("Fill Sweep");
        animFillSweepSix(writeRegister);

        // turn on RGB strip after white animation finishes
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Blue;
        }
        FastLED.show();

        ranOnce = true;
    }
}