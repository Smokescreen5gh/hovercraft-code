#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Animations.h"

#define DATA_PIN   25
#define CLOCK_PIN  32
#define LATCH_PIN  33

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define SCREEN_ADDR  0x3C

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

void showCountdown(const char* nextName, int secondsLeft)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Next Animation:");

    display.setTextSize(2);
    display.setCursor(0, 14);
    display.println(nextName);

    display.setTextSize(1);
    display.setCursor(0, 48);
    display.print("Starting in ");
    display.print(secondsLeft);
    display.println("s");

    display.display();
}

void waitWithCountdown(const char* nextName, int totalSeconds)
{
    for (int i = totalSeconds; i > 0; i--)
    {
        showCountdown(nextName, i);
        delay(1000);
    }
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
        ranOnce = true;
    }
}