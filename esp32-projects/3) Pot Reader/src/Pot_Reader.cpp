#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Define Pins for OLED I2C Protocol
#define SDA_PIN 21
#define SCL_PIN 22

// Define Pin 2 for internal LED
#define LED 2

// Pot pins 
#define POT1_PIN 26
#define POT2_PIN 27

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void ledblinker() {
digitalWrite(LED, HIGH);
  delay(3000);
  
digitalWrite(LED, LOW);
  delay(100);

digitalWrite(LED, HIGH);
delay(100);

digitalWrite(LED, LOW);
delay(100);
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED, OUTPUT);

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
  display.println("Pot Reader Ready");

  // Displays it to the screen
  display.display();  // <-- actually updates screen

  Serial.println("OLED initialized");
}

void loop() {
  //ledblinker();

  // Read the Potentiometer Reading
  int pot1 = analogRead(POT1_PIN);
  int pot2 = analogRead(POT2_PIN);

  // -- Serial Monitor Output --
  Serial.print("POT 1: ");
  Serial.print(pot1);
  Serial.print(" | POT 2: ");
  Serial.println(pot2);

  // ---- Display to the Oled Display----
  display.clearDisplay();

   display.setCursor(0, 0);
  display.print("POT 1: ");
  display.println(pot1);

  display.setCursor(0, 16);
  display.print("POT 2: ");
  display.println(pot2);

  display.display();
  delay(5);

}


