// Import Libraries 
#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP32Servo.h>

// Custom Classes
#include "NRF_Radio.h"
#include "bldc.h"


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

// --------- Pin Definitions for Brushless Motors ----------
#define motor_1 4
#define motor_2 16

static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

// --------- Creating Objects ----------

// Create radio object (CE, CSN)
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);

// Create Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create BLDC Object
BLDC motor1(motor_1, "M1", 1000, 2000);
BLDC motor2(motor_2, "M1", 1000, 2000);

// ---------- Received Pot Values -------
static uint16_t pot1Rx;
static uint16_t pot2Rx;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Prints a text to Serial Monitor
  Serial.println("Starting NRF24 Reciever...");

  // Servo init 
  motor1.begin();
  motor2.begin();

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

    // Only update pot values when we receive a POT packet
    if (gotPacket && in.type == PacketType::POT) {
      pot1Rx = in.pot1Raw;
      pot2Rx = in.pot2Raw;
    }



  // 3) Write PWM Signal from Transmitter to motor
  int throttle1 = map(pot1Rx, 0, 4095, 1000, 2000);
  int throttle2 = map(pot2Rx, 0, 4095, 1000, 2000);

  if (radio.isConnected()) {
    motor1.arm(throttle1);
    motor1.setThrottle(throttle1);

    motor2.arm(throttle2);
    motor2.setThrottle(throttle2);
  }
  else {
    motor1.disable();
    motor2.disable();
  }

  // 4) Send Telemetery to Transmitter
  static uint32_t lastStatusMs = 0;
  if (millis() - lastStatusMs >= 20) {
    lastStatusMs = millis();

    RadioPayload out{};
    out.type = PacketType::MOTOR_STATUS;
    out.counter = 0;     // you don’t really care anymore
    out.motor1State = motor1.getState();
    out.motor2State = motor2.getState();

    radio.sendPackage(out);
  }

  // --------------------- Display on OLED Screen ------------------------
  static uint32_t lastOledMs = 0;

  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // ----- Line 1 -----
    display.setCursor(0, 0);
    display.print("NRF Module Rec");

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
    display.print(motor1.name);
    display.print(":");
    display.print(motor1.getThrottle());
    display.print(" ");

    display.setCursor(50, 28);
    display.print(motor1.getState());

    // ----- Line 4 -----
    display.setCursor(0, 38);
    display.print(motor2.name);
    display.print(":");
    display.print(motor2.getThrottle());
    display.print(" ");

    display.setCursor(50, 38);
    display.print(motor2.getState());

    
    display.display();
  }
}