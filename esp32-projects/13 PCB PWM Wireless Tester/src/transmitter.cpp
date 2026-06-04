// Import Libraries 
#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Custom Classes
#include "NRF_Radio.h"
#include "POT.h"

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


// --------- Pin Definitions for Potentiometers ----------
#define POT1_PIN 33
#define POT2_PIN 25
#define POT3_PIN 26
#define POT4_PIN 27
#define POT5_PIN 14




// --------- Creating Objects ----------

// Create radio object (CE, CSN)
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 1);

// Create Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create Potentiometer Objects
Potentiometer pot1(POT1_PIN, "POT 1");
Potentiometer pot2(POT2_PIN, "POT 2");
Potentiometer pot3(POT3_PIN, "POT 2");
Potentiometer pot4(POT4_PIN, "POT 2");
Potentiometer pot5(POT5_PIN, "POT 2");
static char motor1State = 'D';
static char motor2State = 'D';
static char motor3State = 'D';
static char motor4State = 'D';
static char motor5State = 'D';

void setup() {
  Serial.begin(115200);
  delay(200);

  // Prints a text to Serial Monitor
  Serial.println("Starting NRF24 Transmitter...");

  // Potentiometer setup
  pot1.begin();
  pot2.begin();
  pot3.begin();
  pot4.begin();
  pot5.begin();

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

    if (gotPacket && in.type == PacketType::MOTOR_STATUS) {
      motor1State = in.motor1State;
      motor2State = in.motor2State;
      motor3State = in.motor3State;
      motor4State = in.motor4State;
      motor5State = in.motor5State;
    }

  // 3) Read potentiometers data
  pot1.update();
  pot2.update();
  pot3.update();
  pot4.update();
  pot5.update();


  // 4) Send POT data periodically (50 Hz = every 20 ms)
  static uint32_t lastPotMs = 0;
  if (millis() - lastPotMs >= 20) {
    lastPotMs = millis();

    RadioPayload out{};
    out.type = PacketType::POT;
    out.counter = 0;     // you don’t really care anymore
    out.pot1Raw = pot1.getValue();
    out.pot2Raw = pot2.getValue();
    out.pot3Raw = pot3.getValue();
    out.pot4Raw = pot4.getValue();
    out.pot5Raw = pot5.getValue();

    radio.sendPackage(out);
  }

// 5) Display on OLED Screen
  static uint32_t lastOledMs = 0;

  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Line 1
    display.setCursor(0, 0);
    display.print("NRF TX: ");
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
    display.print("P1:");
    display.setCursor(18, 22);
    display.printf("%4d", pot1.getValue());
    display.setCursor(44, 22);
    display.print(motor1State);

    display.setCursor(64, 22);
    display.print("P2:");
    display.setCursor(82, 22);
    display.printf("%4d", pot2.getValue());
    display.setCursor(108, 22);
    display.print(motor2State);

    // Line 4
    display.setCursor(0, 34);
    display.print("P3:");
    display.setCursor(18, 34);
    display.printf("%4d", pot3.getValue());
    display.setCursor(44, 34);
    display.print(motor3State);

    display.setCursor(64, 34);
    display.print("P4:");
    display.setCursor(82, 34);
    display.printf("%4d", pot4.getValue());
    display.setCursor(108, 34);
    display.print(motor4State);

    // Line 5
    display.setCursor(0, 46);
    display.print("P5:");
    display.setCursor(18, 46);
    display.printf("%4d", pot5.getValue());
    display.setCursor(44, 46);
    display.print(motor5State);

    display.display();
  }

}