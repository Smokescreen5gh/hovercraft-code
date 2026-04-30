#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "NRF_Radio.h"
#include "bldc.h"
#include "FanTester.h"

// NRF24L01 pins
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   5
#define PIN_CE    17

// OLED + Static sensor bus
#define SDA0 21
#define SCL0 22

// Venturi sensor second I2C bus
#define SDA1 25
#define SCL1 26

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

// BLDC motor ESC signal pin
#define MOTOR_PIN 4

// Define Radio Read and Write Address
static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

TwoWire I2C_2 = TwoWire(1);

//------------------------------------------------//
// ------------ Create the Objects ---------------//
//------------------------------------------------//

//Create Radio Object
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);

// Create the Display Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create the motor object
BLDC motor(MOTOR_PIN, "M1", 1000, 2000);

// Need Help
FanTester fanTester(Wire, I2C_2);

static uint16_t potRx = 0;
static uint16_t throttleUs = 1000;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting NRF24 Receiver + Fan Tester...");

  Wire.begin(SDA0, SCL0);
  I2C_2.begin(SDA1, SCL1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Fan Tester RX");
  display.println("Zeroing sensors...");
  display.display();

  motor.begin();

  fanTester.begin();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Zeroing done");
  display.display();
  delay(1000);

  radio.begin();
  Serial.println("Radio initialized.");
}

void loop() {
  radio.serviceConnection();

  RadioPayload in{};
  bool gotPacket = radio.receivePackage(in);

  if (gotPacket && in.type == PacketType::CONTROL) {
    potRx = in.potRaw;
    throttleUs = in.throttleUs;
  }

  if (radio.isConnected()) {
    motor.arm(throttleUs);
    motor.setThrottle(throttleUs);
  } else {
    motor.disable();
    throttleUs = 1000;
  }

  static uint32_t lastSensorMs = 0;
  if (millis() - lastSensorMs >= 50) {
    lastSensorMs = millis();
    fanTester.update();
  }

  static uint32_t lastTelemetryMs = 0;
  if (millis() - lastTelemetryMs >= 50) {
    lastTelemetryMs = millis();

    RadioPayload out{};
    out.type = PacketType::TELEMETRY;
    out.potRaw = potRx;
    out.throttleUs = throttleUs;
    out.staticPa = fanTester.getStaticPressurePa();
    out.venturiPa = fanTester.getVenturiPressurePa();
    out.cfm = fanTester.getCFM();
    out.motorState = motor.getState();

    radio.sendPackage(out);
  }

  static uint32_t lastSerialMs = 0;
  if (millis() - lastSerialMs >= 250) {
    lastSerialMs = millis();

    Serial.print("PotRX: ");
    Serial.print(potRx);
    Serial.print(" | PWM: ");
    Serial.print(throttleUs);
    Serial.print(" | Static: ");
    Serial.print(fanTester.getStaticPressurePa(), 1);
    Serial.print(" Pa | Venturi: ");
    Serial.print(fanTester.getVenturiPressurePa(), 1);
    Serial.print(" Pa | CFM: ");
    Serial.print(fanTester.getCFM(), 1);
    Serial.print(" | Motor: ");
    Serial.println(motor.getState());
  }

  static uint32_t lastOledMs = 0;
  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("NRF Module RX");

    display.setCursor(0, 9);
    display.print(radio.isConnected() ? "Connected" : "Disconnected");

    display.setCursor(0, 18);
    display.print("SA:");
    display.write((const char*)RADIO_TX_ADDR, 5);
    display.print(" RA:");
    display.write((const char*)RADIO_RX_ADDR, 5);

    display.setCursor(0, 28);
    display.print("PWM:");
    display.print(motor.getThrottle());
    display.print(" M:");
    display.print(motor.getState());

    display.setCursor(0, 38);
    display.print("S:");
    display.print(fanTester.getStaticPressurePa(), 1);
    display.print(" V:");
    display.print(fanTester.getVenturiPressurePa(), 1);

    display.setCursor(0, 48);
    display.print("CFM:");
    display.print(fanTester.getCFM(), 1);

    display.display();
  }
}