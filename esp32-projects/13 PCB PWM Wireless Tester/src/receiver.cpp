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
#define PIN_CSN   17
#define PIN_CE    5

// --------- Pin Definitions for OLED Screen ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// --------- Pin Definitions for Brushless Motors ----------
#define motor_1 32
#define motor_2 33
#define motor_3 25
#define motor_4 26
#define motor_5 12


static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

// --------- Creating Objects ----------

// Create radio object (CE, CSN)
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);

// Create Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create BLDC Object
BLDC motor1(motor_1, "M1", 1000, 2000);
BLDC motor2(motor_2, "M2", 1000, 2000);
BLDC motor3(motor_3, "M3", 1000, 2000);
BLDC motor4(motor_4, "M4", 1000, 2000);
BLDC motor5(motor_5, "M4", 1000, 2000);

// ---------- Received Pot Values -------
static uint16_t pot1Rx;
static uint16_t pot2Rx;
static uint16_t pot3Rx;
static uint16_t pot4Rx;
static uint16_t pot5Rx;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Prints a text to Serial Monitor
  Serial.println("Starting NRF24 Reciever...");

  // Servo init 
  motor1.begin();
  motor2.begin();
  motor3.begin();
  motor4.begin();
  motor5.begin();


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
      pot3Rx = in.pot3Raw;
      pot4Rx = in.pot4Raw;
      pot5Rx = in.pot5Raw;
    }



  // 3) Write PWM Signal from Transmitter to motor
  int throttle1 = map(pot1Rx, 0, 4095, 1000, 2000);
  int throttle2 = map(pot2Rx, 0, 4095, 1000, 2000);
  int throttle3 = map(pot3Rx, 0, 4095, 1000, 2000);
  int throttle4 = map(pot4Rx, 0, 4095, 1000, 2000);
  int throttle5 = map(pot5Rx, 0, 4095, 1000, 2000);

  if (radio.isConnected()) {
    motor1.arm(throttle1);
    motor1.setThrottle(throttle1);

    motor2.arm(throttle2);
    motor2.setThrottle(throttle2);

    motor3.arm(throttle3);
    motor3.setThrottle(throttle3);

    motor4.arm(throttle4);
    motor4.setThrottle(throttle4);

    motor5.arm(throttle5);
    motor5.setThrottle(throttle5);

    
  }
  else {
    motor1.disable();
    motor2.disable();
    motor3.disable();
    motor4.disable();
    motor5.disable();
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
    out.motor3State = motor3.getState();
    out.motor4State = motor4.getState();
    out.motor5State = motor5.getState();

    radio.sendPackage(out);
  }

// 5) --------------------- Display on OLED Screen ------------------------
// Display on OLED Screen
  static uint32_t lastOledMs = 0;

  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Line 1
    display.setCursor(0, 0);
    display.print("NRF RX: ");
    display.print(radio.isConnected() ? "CON" : "DIS");

    // Line 2
    display.setCursor(0, 10);
    display.print("SA:");
    display.write((const char*)RADIO_TX_ADDR, 5);

    display.setCursor(60, 10);
    display.print("RA:");
    display.write((const char*)RADIO_RX_ADDR, 5);

    // Line 3
    display.setCursor(0, 22);
    display.print("M1:");
    display.setCursor(18, 22);
    display.printf("%4d", motor1.getThrottle());
    display.setCursor(44, 22);
    display.print(motor1.getState());

    display.setCursor(64, 22);
    display.print("M2:");
    display.setCursor(82, 22);
    display.printf("%4d", motor2.getThrottle());
    display.setCursor(108, 22);
    display.print(motor2.getState());

    // Line 4
    display.setCursor(0, 34);
    display.print("M3:");
    display.setCursor(18, 34);
    display.printf("%4d", motor3.getThrottle());
    display.setCursor(44, 34);
    display.print(motor3.getState());

    display.setCursor(64, 34);
    display.print("M4:");
    display.setCursor(82, 34);
    display.printf("%4d", motor4.getThrottle());
    display.setCursor(108, 34);
    display.print(motor4.getState());

    // Line 5
    display.setCursor(0, 46);
    display.print("M5:");
    display.setCursor(18, 46);
    display.printf("%4d", motor5.getThrottle());
    display.setCursor(44, 46);
    display.print(motor5.getState());

    display.display();
  }
}