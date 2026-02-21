#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initializes the I2C Hardware
  Wire.begin(SDA_PIN, SCL_PIN);

  // Writes a status to Serial Monitor
  Serial.println("Initializing OLED...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found at 0x3C");
    while (true); // Stop here if failed
  }

  // Clears the Display
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Sets Cursor Location
  display.setCursor(10, 20);

  //What to write in the display
  display.println("Hello World");

  // Displays it to the screen
  display.display();  // <-- actually updates screen

  Serial.println("OLED initialized");
}

void loop() {
}