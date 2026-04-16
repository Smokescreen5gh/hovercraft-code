#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include "POT.h"

#define DATA_PIN   25
#define CLOCK_PIN  32
#define LATCH_PIN  33

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDR    0x3C

#define RGB_PIN      26
#define NUM_LEDS     30

#define POT1_PIN     27
#define TOGGLE_PIN   14

CRGB leds[NUM_LEDS];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Potentiometer pot1(POT1_PIN, "POT 1");

// Verify these with hardware test
const uint8_t HEADLIGHTS_OFF_PATTERN = 0x00;
const uint8_t HEADLIGHTS_ON_PATTERN  = 0xFF;

bool lastSwitchState = HIGH;
bool headlightsEnabled = false;

bool animActive = false;
uint8_t animStep = 0;
unsigned long lastAnimMs = 0;
const unsigned long animInterval = 100;

uint16_t potValue = 0;
uint8_t hueValue = 0;

void writeRegister(uint8_t value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

void updateOLED()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Headlights: ");
    display.println(headlightsEnabled ? "ON" : "OFF");

    display.setCursor(0, 16);
    display.print("Pot Raw: ");
    display.println(potValue);

    display.setCursor(0, 32);
    display.print("RGB Hue: ");
    display.println(hueValue);

    display.setCursor(0, 48);
    display.print("Anim: ");
    display.println(animActive ? "RUNNING" : "IDLE");

    display.display();
}

void updateRGBFromPot()
{
    pot1.update();
    potValue = pot1.getValue();
    hueValue = map(potValue, 0, 4095, 0, 255);

    fill_solid(leds, NUM_LEDS, CHSV(hueValue, 255, 255));
    FastLED.show();
}

void startHeadlightAnimation()
{
    animActive = true;
    animStep = 0;
    lastAnimMs = millis();
    headlightsEnabled = true;
}

void turnHeadlightsOff()
{
    animActive = false;
    headlightsEnabled = false;
    writeRegister(HEADLIGHTS_OFF_PATTERN);
}

void updateHeadlightAnimation()
{
    if (!animActive) return;

    unsigned long now = millis();
    if (now - lastAnimMs < animInterval) return;

    lastAnimMs = now;

    // Example 6-step fill animation
    // You may need to invert these depending on your hardware
    switch (animStep)
    {
        case 0: writeRegister(0b11111110); break;
        case 1: writeRegister(0b11111100); break;
        case 2: writeRegister(0b11111000); break;
        case 3: writeRegister(0b11110000); break;
        case 4: writeRegister(0b11100000); break;
        case 5: writeRegister(0b11000000); break;
        default:
            writeRegister(HEADLIGHTS_ON_PATTERN);
            animActive = false;
            return;
    }

    animStep++;
}

void setup()
{
    Serial.begin(115200);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(TOGGLE_PIN, INPUT_PULLUP);

    writeRegister(HEADLIGHTS_OFF_PATTERN);

    Wire.begin(21, 22);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR))
    {
        while (true) {}
    }

    pot1.begin();

    FastLED.addLeds<WS2812B, RGB_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.setBrightness(80);
    FastLED.show();
}

void loop()
{
    updateRGBFromPot();

    bool switchState = digitalRead(TOGGLE_PIN);

    // current assumption:
    // LOW = switch active
    // HIGH = switch inactive

    if (lastSwitchState == HIGH && switchState == LOW)
    {
        startHeadlightAnimation();
    }

    if (switchState == HIGH)
    {
        turnHeadlightsOff();
    }

    updateHeadlightAnimation();
    updateOLED();

    lastSwitchState = switchState;
    delay(10);
}