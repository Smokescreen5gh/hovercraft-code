#include <Arduino.h>
#include <ESP32Servo.h>
#include <Adafruit_PWMServoDriver.h>

/*
 --------------------------------------------------------------
       Import Libraries
 --------------------------------------------------------------
*/
// Oled Library
#include "DisplayManager.h"      // Hardware Library
#include "Reciever_Info.h" // Data Library

// Radio Library
#include "RadioPayload.h" // Data Library
#include "NRF_Radio.h"    // Hardware Library


// Output Devices Libraries
#include "bldc.h"
#include "PCA_Servo.h"



// I2C Buses
// BUS #1: OLED + Static sensor bus
#define SDA1 21
#define SCL1 22

// BUS #2: Venturi sensor second I2C bus
#define SDA2 27
#define SCL2 14

TwoWire I2C_0 = TwoWire(0);
TwoWire I2C_1 = TwoWire(1);

/*
 --------------------------------------------------------------
       Generate Objects
 --------------------------------------------------------------
*/
// ------- PWM Driver ---------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, I2C_1);

// ------- Display Object -----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

DisplayManager oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_I2C_ADDRESS, I2C_0, SDA1, SCL1); // Create Oled Object
Reciever_Info reciever_info(oled_display); // Fill OLED object With Reciever Information


// ------- NRF Radio Object -----------
// --------- Pin Definitions for NRF2401L ----------
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   17
#define PIN_CE    5

static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

// Create radio object (CE, CSN)
NrfRadio<RadioPayload> radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);

// ------- BLDC Motors -----------


// ------- Servos -----------
PCA_Servo TopRight_Servo(pwm, 6);
PCA_Servo TopLeft_Servo(pwm, 7);
PCA_Servo BottomRight_Servo(pwm, 8);
PCA_Servo BottomLeft_Servo(pwm, 9);


// ------- Persistent Received Data -----------
// Stores the LAST valid CONTROL packet
RadioPayload controlRx{};


void setup() {
    Serial.begin(115200);
    delay(200);

    oled_display.begin("RX Servo Control");

    // Initialize the I2C Buses
    //I2C_0.begin(SDA1, SCL1);
    I2C_1.begin(SDA2, SCL2);

    if (!pwm.begin()) {
    Serial.println("PCA9685 FAILED");
    }

    else {
        Serial.println("PCA9685 CONNECTED");
    }

    pwm.setPWMFreq(50);

    // Intialize the Radio
    radio.begin();

    // Initalize the Servos
    TopRight_Servo.begin(0);
    TopLeft_Servo.begin(0);
    BottomRight_Servo.begin(0);
    BottomLeft_Servo.begin(0);
}

void loop() {
    unsigned long nowMs = millis();


    // ======================================================
    // 1) Maintain heartbeat + timeout
    // ======================================================

    radio.serviceConnection();


    // ======================================================
    // 2) Receive packets
    // ======================================================

    RadioPayload in{};

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
    // 3) Control the servos
    // ======================================================
    uint8_t servoAngle = map(controlRx.pot1, 0, 4095, 0, 180);
    TopRight_Servo.write(servoAngle);
    TopLeft_Servo.write(servoAngle);
    BottomRight_Servo.write(servoAngle);
    BottomLeft_Servo.write(servoAngle);

    // ======================================================
    // 4) Send Telemetry every 500 ms
    // ======================================================

    static uint32_t lastTelemetryMs = 0;
    if (nowMs - lastTelemetryMs >= 500)
    {
        lastTelemetryMs = nowMs;

    
        RadioPayload out{};

        out.type = PacketType::TELEMETRY;

        out.Servo_1_Angle = servoAngle;
        out.Servo_2_Angle = servoAngle;
        out.Servo_3_Angle = servoAngle;
        out.Servo_4_Angle = servoAngle;
       
        radio.sendPackage(out);
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

            servoAngle,
            servoAngle,
            servoAngle,
            servoAngle,
            controlRx.pot1,
            radio.isConnected()
        );
    }

    delay(5);

}