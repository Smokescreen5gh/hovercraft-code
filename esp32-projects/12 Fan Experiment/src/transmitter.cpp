#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "NRF_Radio.h"
#include "POT.h"

// NRF24 pins
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   5
#define PIN_CE    17

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// Potentiometer
#define POT_PIN 27

static const uint8_t RADIO_RX_ADDR[6] = "00001";
static const uint8_t RADIO_TX_ADDR[6] = "00002";

NrfRadio radio(PIN_CE, PIN_CSN,
               RADIO_RX_ADDR,
               RADIO_TX_ADDR,
               1);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET);

Potentiometer pot(POT_PIN, "Throttle Pot");

static uint16_t throttleUs = 1000;

static float rxStaticPa = 0.0f;
static float rxVenturiPa = 0.0f;

static float rxFlowM3s = 0.0f;
static float rxV1 = 0.0f;
static float rxV2 = 0.0f;

static char rxMotorState = 'D';

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println("Starting NRF24 Transmitter...");

  pot.begin();

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          OLED_I2C_ADDRESS))
  {
    while (true) delay(1000);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0,0);
  display.println("Fan Tester TX");

  display.display();

  delay(1000);

  radio.begin();

  Serial.println("Radio initialized.");
}

void loop()
{
  radio.serviceConnection();

  RadioPayload in{};

  bool gotPacket =
      radio.receivePackage(in);

  if (gotPacket &&
      in.type ==
      PacketType::TELEMETRY)
  {
    rxStaticPa  = in.staticPa;
    rxVenturiPa = in.venturiPa;

    rxFlowM3s = in.flowM3s;

    rxV1 = in.v1;
    rxV2 = in.v2;

    rxMotorState = in.motorState;

    throttleUs = in.throttleUs;
  }

  pot.update();

  uint16_t potRaw =
      pot.getValue();

  throttleUs =
      map(potRaw,
          0,
          4095,
          1000,
          2000);

  static uint32_t lastControlMs = 0;

  if (millis() - lastControlMs >= 20)
  {
    lastControlMs = millis();

    RadioPayload out{};

    out.type =
        PacketType::CONTROL;

    out.potRaw =
        potRaw;

    out.throttleUs =
        throttleUs;

    radio.sendPackage(out);
  }

  static uint32_t lastSerialMs = 0;

  if (millis() - lastSerialMs >= 250)
  {
    lastSerialMs = millis();

    Serial.print("Pot: ");
    Serial.print(potRaw);

    Serial.print(" | PWM: ");
    Serial.print(throttleUs);

    Serial.print(" | Static: ");
    Serial.print(rxStaticPa,1);
    Serial.print(" Pa");

    Serial.print(" | dP: ");
    Serial.print(rxVenturiPa,1);
    Serial.print(" Pa");

    Serial.print(" | Q: ");
    Serial.print(rxFlowM3s,4);
    Serial.print(" m3/s");

    Serial.print(" | V1: ");
    Serial.print(rxV1,2);
    Serial.print(" m/s");

    Serial.print(" | V2: ");
    Serial.print(rxV2,2);
    Serial.print(" m/s");

    Serial.print(" | Motor: ");
    Serial.println(rxMotorState);
  }

  static uint32_t lastOledMs = 0;

  if (millis() - lastOledMs >= 100)
  {
    lastOledMs = millis();

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0,0);
    display.print("NRF TX ");

    display.print(
        radio.isConnected()
        ? "CON"
        : "DISC");

    display.setCursor(0,10);
    display.print("Pot:");
    display.print(potRaw);

    display.setCursor(0,20);
    display.print("S:");
    display.print(rxStaticPa,0);

    display.print(" dP:");
    display.print(rxVenturiPa,0);

    display.setCursor(0,30);
    display.print("Q:");
    display.print(rxFlowM3s,3);

    display.setCursor(0,40);
    display.print("V1:");
    display.print(rxV1,1);

    display.print(" V2:");
    display.print(rxV2,1);

    display.setCursor(0,50);
    display.print("Motor:");
    display.print(rxMotorState);

    display.display();
  }
}