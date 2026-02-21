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

// Define pins for Pushbuttons
#define Button_1 32
#define Button_2 33

// Define External LED pins
#define LED_1 17
#define LED_2 16


// --------------------------- Global Objects ---------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool pressswitch(int pin, int x_pos, int y_pos, const char* name) {

  byte buttonState = digitalRead(pin);

  display.setCursor(x_pos, y_pos);
  display.print(name);
  display.print(":");

  if (buttonState == HIGH) {
    display.print("ON");
    return true;
  }

  else {
    display.print("OFF");
    return false;
  }
}

bool toggleswitch(int pin, int x_pos, int y_pos, const char* name) {

  static bool toggleState[40] = {false};   // remembers ON/OFF per pin
  static bool lastState[40]   = {HIGH};    // remembers last reading per pin

  bool currentState = digitalRead(pin);

  // Detect press event (HIGH -> LOW)
  if (lastState[pin] == HIGH && currentState == LOW) {
    toggleState[pin] = !toggleState[pin];
  }

  lastState[pin] = currentState;

  display.setCursor(x_pos, y_pos);
  display.print(name);
  display.print(":");

  if (toggleState[pin] == true) {
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

  Serial.println("OLED initialized");

  // Setup Buttons
  pinMode(Button_1, INPUT);
  pinMode(Button_2, INPUT);

  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, LOW);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, LOW);

}


void loop() {

  display.clearDisplay();

  if (toggleswitch(Button_1, 0, 16, "Button 1")) {
    digitalWrite(LED_1, HIGH);
  }
  else {
    digitalWrite(LED_1, LOW);
  }

  if (toggleswitch(Button_2, 0, 26, "Button 2")) {
    digitalWrite(LED_2, HIGH);
  }
  else {
    digitalWrite(LED_2, LOW);
  }

  display.display();
}

 
