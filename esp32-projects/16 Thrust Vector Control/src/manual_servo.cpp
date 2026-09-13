#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Oled Library
#include "DisplayManager.h"      // Hardware Library
#include "Step1_Display.h"      // Data Library

// PCA Library Servo
#include "PCA_Servo.h"

// I2C Assignments
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

// ------- Display Object -----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

DisplayManager oled_display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    OLED_RESET,
    OLED_I2C_ADDRESS,
    I2C_0,
    SDA1,
    SCL1
);

Step1_Display step1_display(oled_display);


// ------- PWM Driver ---------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, I2C_1);


// ------- Servos -----------
PCA_Servo servo1(pwm, 0);
    // Servo angle variable
    int servo1_angle = 0;
PCA_Servo servo2(pwm, 1);
    // Servo angle variable
    int servo2_angle = 0;


/*
 --------------------------------------------------------------
       Serial Command Function
 --------------------------------------------------------------
*/

void checkSerialCommand()
{
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();


        // ======================================================
        // Servo 1 Command
        // Example: s1 45
        // ======================================================

        if (cmd.startsWith("s1 "))
        {
            servo1_angle = cmd.substring(3).toInt();

            // Check that angle is within servo range
            if (servo1_angle >= 0 && servo1_angle <= 180)
            {
                servo1.write(servo1_angle);

                Serial.print("Servo 1 moved to: ");
                Serial.print(servo1_angle);
                Serial.println(" degrees");
            }
            else
            {
                Serial.println("Invalid Servo 1 angle.");
                Serial.println("Enter a value from 0 to 180.");
            }
        }

        else if (cmd.startsWith("s2 "))
        {
            servo2_angle = cmd.substring(3).toInt();

            // Check that angle is within servo range
            if (servo2_angle >= 0 && servo2_angle <= 180)
            {
                servo2.write(servo2_angle);

                Serial.print("Servo 2 moved to: ");
                Serial.print(servo2_angle);
                Serial.println(" degrees");
            }
            else
            {
                Serial.println("Invalid Servo 2 angle.");
                Serial.println("Enter a value from 0 to 180.");
            }
        }


        // ======================================================
        // Unknown Command
        // ======================================================

        else
        {
            Serial.println("Unknown command.");
            Serial.println("Example: s1 90");
        }
    }
}


/*
 --------------------------------------------------------------
       Setup
 --------------------------------------------------------------
*/

void setup()
{
    Serial.begin(115200);

    // Start I2C bus first
    I2C_0.begin(SDA1, SCL1);
    I2C_1.begin(SDA2, SCL2);

    // Start OLED
    oled_display.begin("Thruster Servo Control");
    delay(2000);

    // Intialize the PCA9685 Board
    if (!pwm.begin()) {
    Serial.println("PCA9685 FAILED");
    }

    else {
        Serial.println("PCA9685 CONNECTED");
    }
    pwm.setPWMFreq(50);

    // Start servo at 0 degrees
    servo1.begin(90);
    servo2.begin(85);

    Serial.println();
    Serial.println("=================================");
    Serial.println("     PCA SERVO TEST PROGRAM");
    Serial.println("=================================");
    Serial.println();
    Serial.println("Servo 1 Command:");
    Serial.println("s1 <angle>");
    Serial.println();
    Serial.println("Example:");
    Serial.println("s1 45");
    Serial.println();
}


/*
 --------------------------------------------------------------
       Main Loop
 --------------------------------------------------------------
*/

void loop()
{
    unsigned long nowMs = millis();

    checkSerialCommand();


    // ======================================================
    // Update OLED every 200 ms
    // ======================================================

    static unsigned long lastDisplayMs = 0;

    if (nowMs - lastDisplayMs >= 200)
    {
        lastDisplayMs = nowMs;

        step1_display.update(
            servo1_angle,
            servo2_angle
        );
    }
}