#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// --------------------------- OLED Configuration ---------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1              // Most I2C SSD1306 modules don't use a reset pin
#define OLED_I2C_ADDRESS 0x3C      // Common addresses: 0x3C or 0x3D

// --------------------------- Pin Definitions ---------------------------
// I2C pins for Display
#define SDA_PIN 21
#define SCL_PIN 22

// Toggle switch (SPDT) on D19 / GPIO19
#define togglepin_1 19


// Define External LED pins
#define LED_1 17
#define LED_2 16


// --------------------------- Global Objects ---------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool togglepin(int x, int x_pos, int y_pos, const char* name) {

  int state = digitalRead(x);

  display.setCursor(x_pos, y_pos);
  display.print(name);
  display.print(":");

  if (state == LOW) {
    display.print("ON");
    return true;
  }

  else {
    display.print("OFF");
    return false;
  }

}

void setup() {
  Serial.begin(115200);
  delay(200); 

  // I2C init
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  Serial.println("Initializing OLED...");
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("SSD1306 not found. Check wiring and I2C address (0x3C/0x3D).");
    while (true) { delay(1000); } // halt if OLED init fails
  }

  // Basic OLED settings
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Startup message (shown once)
  display.setCursor(10, 20);
  display.println("Pot Reader Ready");
  display.display();

  Serial.println("Hello");

  // Setup Buttons
  pinMode(togglepin_1, INPUT_PULLUP);

  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, LOW);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, LOW);

}


void loop() {

  display.clearDisplay();

  if (togglepin(togglepin_1, 0, 16, "Pin 1")){
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
  }

  else {
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
  }

  display.display();

}

 
