#include <Arduino.h>
#include <ESP32Servo.h>
#include <Adafruit_PWMServoDriver.h>

/*
 --------------------------------------------------------------
       Import Libraries
 --------------------------------------------------------------
*/
#include "DisplayManager.h"
#include "Reciever_Info.h"

#include "bldc.h"
#include "Joystick.h"
#include "POT.h"
#include "Toggle.h"

#include "NRF_Radio.h"
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
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   5
#define PIN_CE    17

static const uint8_t RADIO_RX_ADDR[6] = "00002";
static const uint8_t RADIO_TX_ADDR[6] = "00001";

NrfRadio radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 2);

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

    oled_display.begin("Receiver Mark 1");

    // Initialize the I2C Buses
    I2C_0.begin(SDA1, SCL1);
    I2C_1.begin(SDA2, SCL2);

    if (!pwm.begin()) {
    Serial.println("PCA9685 FAILED");
    }

    else {
        Serial.println("PCA9685 CONNECTED");
    }

    pwm.setPWMFreq(50);

    // Initalize the Servos
    TopRight_Servo.begin(90);
    TopLeft_Servo.begin(90);
    BottomRight_Servo.begin(90);
    BottomLeft_Servo.begin(90);
}

void loop() {
    for (int angle = 0; angle <= 180; angle++)
    {
        TopRight_Servo.write(angle);
        TopLeft_Servo.write(angle);
        BottomRight_Servo.write(angle);
        BottomLeft_Servo.write(angle);

        delay(15);
    }

    for (int angle = 180; angle >= 0; angle--)
    {
        TopRight_Servo.write(angle);
        TopLeft_Servo.write(angle);
        BottomRight_Servo.write(angle);
        BottomLeft_Servo.write(angle);

        delay(15);
    }

}