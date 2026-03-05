#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "NRF_Radio.h"

// --------- Pin Definitions for NRF2401L ----------
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   5
#define PIN_CE    17

// --------- Pin Definitions for OLED Screen ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

static const uint8_t RADIO_RX_ADDR[6] = "00001";
static const uint8_t RADIO_TX_ADDR[6] = "00002";

// --------- Creating Objects ----------

// Create radio object (CE, CSN)
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 1);

// Create Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Message to Send -------
static const char* sendMessage = "Hello Board 2!";
static char lastRx[24] = "(none)";

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting NRF24 Transmitter...");

  // Oled Screeen Intializtion
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    while (true) delay(1000);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("NRF Tester");
  display.display();
  delay(2000);

  radio.begin(); 
  Serial.println("Radio initialized.");
}

void loop() {

  // 1) Maintain heartbeat + timeout
  radio.serviceConnection();

  // 2) Check for received packets
  RadioPayload in{};
  bool gotPacket = radio.receivePackage(in);
  if (gotPacket && in.type == PacketType::TEXT) {
    strncpy(lastRx, in.text, sizeof(lastRx));
    lastRx[sizeof(lastRx) - 1] = '\0';
  }

  // 3) Send TEXT every 1 second
  static uint32_t lastTextMs = 0;
  if (millis() - lastTextMs >= 1000) {
    lastTextMs = millis();

    RadioPayload out{};
    out.type = PacketType::TEXT;
    out.counter = 0;   // you don’t really care anymore
    strncpy(out.text, sendMessage, sizeof(out.text));

    radio.sendPackage(out);
  }

  // Display on OLED Screen
  static uint32_t lastOledMs = 0;

  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ----- Line 1 -----
    display.setCursor(0, 0);
    display.print("NRF Module Tester #1");

    // ----- Line 2 -----
    display.setCursor(0, 9);
    display.print(radio.isConnected() ? "Connected" : "Disconnected");

    // ----- Line 3 -----
    display.setCursor(0, 18);
    display.print("SA:");
    display.write((const char*)RADIO_TX_ADDR, 5);
    display.print(" RA:");
    display.write((const char*)RADIO_RX_ADDR, 5);

    // ----- Line 4 -----
    display.setCursor(0, 28);
    display.print("Send: ");
    display.print(sendMessage);

    // ----- Line 5 -----
    display.setCursor(0, 38);
    display.print("Recv: ");
    if (radio.isConnected()) {
      display.print(lastRx);
    } 
    else {
      display.print("(none)");
    }
    display.display();
  }
}