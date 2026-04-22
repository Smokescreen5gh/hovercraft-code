#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ShiftRegisterDriver.h"
#include "Headlight.h"
#include "Toggle.h"
#include "RGB.h"
#include "POT.h"
#include "DisplayManager.h"

// ============================================================
// Pin definitions
// ============================================================
#define SER_DATA   25
#define SER_CLK    32
#define SER_LATCH  33
#define TOGGLE_PIN 14
#define POT_PIN    27

// OLED pins / settings
#define OLED_SDA       21
#define OLED_SCL       22
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDR      0x3C

// ============================================================
// Driver instances
// ============================================================
ShiftRegisterDriver headlightDriver(SER_DATA, SER_CLK, SER_LATCH, 1, "Headlight Registers");

// One rectangle using 6 LEDs on outputs 0 through 5
HeadlightRect rect1(headlightDriver, 6, 0, "Rect 1");

ToggleSwitch lightSwitch(TOGGLE_PIN, "Light Switch");
Potentiometer rgbPot(POT_PIN, "RGB Pot");

// RGB strip on GPIO 26, 8 LEDs
RGBStrip rgbStrip(26, 8, "Main RGB");

DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_ADDR, "OLED");

// ============================================================
// setup
// ============================================================
void setup()
{
    headlightDriver.begin();
    rect1.begin();

    lightSwitch.begin();
    rgbPot.begin();
    rgbStrip.begin();

    displayManager.begin(OLED_SDA, OLED_SCL);

    Adafruit_SSD1306& display = displayManager.getDisplay();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Lighting System");
    display.println("Booting...");
    display.display();

    delay(1200);
}

// ============================================================
// loop
// ============================================================
void loop()
{
    lightSwitch.update();
    rgbPot.update();

    if (lightSwitch.isOn())
    {
        // Start startup animation only if currently OFF
        if (rect1.isOff())
        {
            rect1.startCenterFill(120);
        }

        // Keep RGB strip on and update hue from pot
        if (!rgbStrip.isOn())
        {
            rgbStrip.turnOn();
        }

        rgbStrip.updateFromPot(rgbPot.getValue());
    }
    else
    {
        // Start shutdown animation only if currently fully ON
        if (rect1.isOn())
        {
            rect1.startShutdown(120);
        }

        // Turn RGB strip off
        if (rgbStrip.isOn())
        {
            rgbStrip.turnOff();
        }
    }

    rect1.update();

    Adafruit_SSD1306& display = displayManager.getDisplay();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.print("System: ");
    display.println(lightSwitch.isOn() ? "ON" : "OFF");

    display.print("Headlight: ");
    display.println(rect1.getName());

    display.print("HL State: ");
    if (rect1.isOff())
        display.println("OFF");
    else if (rect1.isAnimating())
        display.println("ANIM");
    else
        display.println("ON");

    display.print("RGB: ");
    display.println(rgbStrip.isOn() ? "ON" : "OFF");

    display.print("Hue: ");
    display.println(rgbStrip.getHue());

    display.display();
}