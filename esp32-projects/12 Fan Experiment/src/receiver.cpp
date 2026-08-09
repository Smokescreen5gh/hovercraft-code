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

// Radio addresses
static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

TwoWire I2C_2 = TwoWire(1);

// ── Objects ──────────────────────────────────────────────────────────────────
NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BLDC motor(MOTOR_PIN, "M1", 1000, 2000);

// Wire  = static pressure sensor bus
// I2C_2 = venturi pressure sensor bus
FanTester fanTester(Wire, I2C_2);

static uint16_t potRx     = 0;
static uint16_t throttleUs = 1000;

// ── Setup ────────────────────────────────────────────────────────────────────
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

// ── Loop ─────────────────────────────────────────────────────────────────────
void loop() {

  radio.serviceConnection();

  // Receive incoming control packets
  RadioPayload in{};
  bool gotPacket = radio.receivePackage(in);

  if (gotPacket && in.type == PacketType::CONTROL) {
    potRx      = in.potRaw;
    throttleUs = in.throttleUs;
  }

  // Drive motor
  if (radio.isConnected()) {
    motor.arm(throttleUs);
    motor.setThrottle(throttleUs);
  } else {
    motor.disable();
    throttleUs = 1000;
  }

  // ── Sensor update @ 50 Hz ──────────────────────────────────────────────────
  static uint32_t lastSensorMs = 0;
  if (millis() - lastSensorMs >= 50) {
    lastSensorMs = millis();
    fanTester.update();
  }

  // ── Telemetry transmit @ 50 Hz ─────────────────────────────────────────────
  static uint32_t lastTelemetryMs = 0;
  if (millis() - lastTelemetryMs >= 50) {
    lastTelemetryMs = millis();

    RadioPayload out{};
    out.type       = PacketType::TELEMETRY;
    out.potRaw     = potRx;
    out.throttleUs = throttleUs;
    out.staticPa   = fanTester.getStaticPressurePa();
    out.venturiPa  = fanTester.getVenturiPressurePa();
    out.flowM3s    = fanTester.getFlowM3s();
    out.v1         = fanTester.getV1();
    out.v2         = fanTester.getV2();
    out.motorState = motor.getState();

    radio.sendPackage(out);
  }

  // ── Serial debug @ 50 Hz (raw, unfiltered) ────────────────────────────────
  static uint32_t lastSerialMs = 0;
  if (millis() - lastSerialMs >= 20) {  // 50 Hz = 20ms intervals
    lastSerialMs = millis();

    Serial.print(millis());
    Serial.print(",");
    Serial.print(fanTester.getStaticPressurePa(), 2);
    Serial.print(",");
    Serial.print(fanTester.getStaticPaFiltered(), 2);
    Serial.print(",");
    Serial.print(fanTester.getVenturiPressurePa(), 2);
    Serial.print(",");
    Serial.print(fanTester.getVenturiPaFiltered(), 2);
    Serial.print(",");
    Serial.print(fanTester.getFlowM3s(), 4);
    Serial.print(",");
    Serial.print(motor.getThrottle());
    Serial.println();
  }

  /* ── Serial debug @ 4 Hz ───────────────────────────────────────────────────
  static uint32_t lastSerialMs = 0;
  if (millis() - lastSerialMs >= 250) {
    lastSerialMs = millis();

    Serial.print("PotRX: ");       Serial.print(potRx);
    Serial.print(" | PWM: ");      Serial.print(throttleUs);
    Serial.print(" | Static: ");   Serial.print(fanTester.getStaticPressurePa(), 1); Serial.print(" Pa");
    Serial.print(" | dP: ");       Serial.print(fanTester.getVenturiPressurePa(), 1); Serial.print(" Pa");
    Serial.print(" | V1: ");       Serial.print(fanTester.getV1(), 2);   Serial.print(" m/s");
    Serial.print(" | V2: ");       Serial.print(fanTester.getV2(), 2);   Serial.print(" m/s");
    Serial.print(" | Flow: ");     Serial.print(fanTester.getFlowM3s(), 4); Serial.print(" m3/s");
    Serial.print(" | S_OK:");      Serial.print(fanTester.staticSensorOK()  ? "Y" : "N");
    Serial.print(" | V_OK:");      Serial.print(fanTester.venturiSensorOK() ? "Y" : "N");
    Serial.print(" | Motor: ");    Serial.println(motor.getState());
  } */

  // ── OLED update @ 10 Hz ───────────────────────────────────────────────────
  static uint32_t lastOledMs = 0;
  if (millis() - lastOledMs >= 100) {
    lastOledMs = millis();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Row 0 — board role + connection status
    display.setCursor(0, 0);
    display.print("NRF RX ");
    display.print(radio.isConnected() ? "CON" : "DISC");

    // Row 1 — send/receive addresses
    display.setCursor(0, 9);
    display.print("SA:");
    display.write((const char*)RADIO_TX_ADDR, 5);
    display.print(" RA:");
    display.write((const char*)RADIO_RX_ADDR, 5);

    // Row 2 — throttle PWM + motor state
    display.setCursor(0, 18);
    display.print("PWM:");
    display.print(motor.getThrottle());
    display.print(" M:");
    display.print(motor.getState());

    // Row 3 — static pressure + venturi deltaP
    display.setCursor(0, 28);
    display.print("S:");
    display.print(fanTester.getStaticPressurePa(), 0);
    display.print(" dP:");
    display.print(fanTester.getVenturiPressurePa(), 0);

    // Row 4 — chamber velocity + throat velocity
    display.setCursor(0, 38);
    display.print("V1:");
    display.print(fanTester.getV1(), 1);
    display.print(" V2:");
    display.print(fanTester.getV2(), 1);

    // Row 5 — volumetric flow
    display.setCursor(0, 48);
    display.print("Q:");
    display.print(fanTester.getFlowM3s(), 4);
    display.print(" m3/s");

    // Row 6 — sensor health
    display.setCursor(0, 57);
    display.print("S:");
    display.print(fanTester.staticSensorOK()  ? "Y" : "N");
    display.print(" V:");
    display.print(fanTester.venturiSensorOK() ? "Y" : "N");

    display.display();
  }
}