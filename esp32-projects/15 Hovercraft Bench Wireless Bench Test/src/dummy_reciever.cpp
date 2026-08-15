#include <Arduino.h>

/*
 --------------------------------------------------------------
       Import Libraries
 --------------------------------------------------------------
*/
// Oled Library
#include "DisplayManager.h"      // Hardware Library
#include "Dummy_Reciever_Info.h" // Data Library

// Radio Library
#include "NRF_Radio.h"          // Hardware Library
#include "Dummy_RadioPayload.h" // Data Library


/*
 --------------------------------------------------------------
       Generate Objects
 --------------------------------------------------------------
*/

// ------- Display Object -----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

TwoWire I2C_0 = TwoWire(0);

DisplayManager oled_display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    OLED_RESET,
    OLED_I2C_ADDRESS,
    I2C_0,
    SDA_PIN,
    SCL_PIN
);

Reciever_Info reciever_info(oled_display);


// ------- NRF Radio Object -----------
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   5
#define PIN_CE    17

static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

NrfRadio<Dummy_RadioPayload> radio(
    PIN_CE,
    PIN_CSN,
    RADIO_RX_ADDR,
    RADIO_TX_ADDR,
    2
);


// ------- Persistent Received Data -----------

// Stores the LAST valid CONTROL packet
Dummy_RadioPayload controlRx{};

// Stores fake telemetry value
static uint8_t randomNumber = 0;


/*
 --------------------------------------------------------------
       Setup
 --------------------------------------------------------------
*/

void setup()
{
    Serial.begin(115200);
    delay(200);

    // Initialize OLED
    oled_display.begin("Hovercraft Receiver Mark 1");

    // Initialize NRF
    radio.begin();

    randomSeed(micros());
}


/*
 --------------------------------------------------------------
       Loop
 --------------------------------------------------------------
*/

void loop()
{
    unsigned long nowMs = millis();


    // ======================================================
    // 1) Maintain heartbeat + timeout
    // ======================================================

    radio.serviceConnection();


    // ======================================================
    // 2) Receive packets
    // ======================================================

    Dummy_RadioPayload in{};

    if (radio.receivePackage(in))
    {
        // Only save CONTROL packets
        if (in.type == PacketType::CONTROL)
        {
            controlRx = in;

            Serial.println("----- CONTROL PACKET -----");

            Serial.print(">J1 X: ");
            Serial.println(controlRx.joy1X);

            Serial.print(">J1 Y: ");
            Serial.println(controlRx.joy1Y);

            Serial.print(">J1 B: ");
            Serial.println(controlRx.joy1Button);

            Serial.print(">J2 X: ");
            Serial.println(controlRx.joy2X);

            Serial.print(">J2 Y: ");
            Serial.println(controlRx.joy2Y);

            Serial.print(">J2 B: ");
            Serial.println(controlRx.joy2Button);

            Serial.print(">P1: ");
            Serial.println(controlRx.pot1);

            Serial.print(">P2: ");
            Serial.println(controlRx.pot2);

            Serial.print(">P3: ");
            Serial.println(controlRx.pot3);

            Serial.print(">S1: ");
            Serial.println(controlRx.switch1);

            Serial.print(">S2: ");
            Serial.println(controlRx.switch2);

            Serial.print(">S3: ");
            Serial.println(controlRx.switch3);

            Serial.println();
        }
    }


    // ======================================================
    // 3) Send fake telemetry every 500 ms
    // ======================================================

    static uint32_t lastTelemetryMs = 0;

    if (nowMs - lastTelemetryMs >= 500)
    {
        lastTelemetryMs = nowMs;

        randomNumber = random(1, 101);

        Dummy_RadioPayload out{};

        out.type = PacketType::TELEMETRY;
        out.randomNumber = randomNumber;

        radio.sendPackage(out);

        Serial.print("Sent random telemetry: ");
        Serial.println(randomNumber);
    }


    // ======================================================
    // 4) Update OLED every 200 ms
    // ======================================================

    static unsigned long lastDisplayMs = 0;

    if (nowMs - lastDisplayMs >= 200)
    {
        lastDisplayMs = nowMs;

        reciever_info.update(
            (const char*)RADIO_TX_ADDR,
            (const char*)RADIO_RX_ADDR,

            controlRx.joy1X,
            controlRx.joy1Y,
            controlRx.joy1Button,

            controlRx.joy2X,
            controlRx.joy2Y,
            controlRx.joy2Button,

            controlRx.pot1,
            controlRx.pot2,
            controlRx.pot3,

            controlRx.switch1,
            controlRx.switch2,
            controlRx.switch3,

            radio.isConnected(),
            randomNumber
        );
    }

    delay(5);
}