#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Oled Library
#include "DisplayManager.h"      // Hardware Library
#include "Step4_Display.h"      // Data Library

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

Step4_Display step4_display(oled_display);


// ------- PWM Driver ---------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, I2C_1);


// ------- Servos -----------
PCA_Servo servo1(pwm, 0);
    // Servo Calibration
    int servo1_center_raw = 94;
    int servo1_range = 16;

    // Relative Corrdinate Frame
    int servo1_relative_angle = 0;
    int servo1_min_relative = -servo1_range;
    int servo1_max_relative = servo1_range;

    // Raw servo command
    int servo1_raw_angle = servo1_center_raw + servo1_relative_angle;

PCA_Servo servo2(pwm, 1);
    // Servo Calibration
    int servo2_center_raw = 85;
    int servo2_range = 25;

    // Relative Corrdinate Frame
    int servo2_relative_angle = 0;
    int servo2_min_relative = -servo2_range;
    int servo2_max_relative = servo2_range;

    // Raw servo command
    int servo2_raw_angle = servo2_center_raw + servo2_relative_angle;


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
            int requestedAngle = cmd.substring(3).toInt();

            if (requestedAngle >= servo1_min_relative && requestedAngle <= servo1_max_relative)
            {
                servo1_relative_angle = requestedAngle;

                Serial.print("Servo 1 command set to: ");
                Serial.print(servo1_relative_angle);
                Serial.println(" degrees");
            }
            else
            {
                Serial.println("Invalid Servo 1 angle.");
            }
        }

        else if (cmd.startsWith("s2 "))
        {
            int requestedAngle = cmd.substring(3).toInt();

            if (requestedAngle >= servo2_min_relative && requestedAngle <= servo2_max_relative)
            {
                servo2_relative_angle = requestedAngle;

                Serial.print("Servo 2 command set to: ");
                Serial.print(servo2_relative_angle);
                Serial.println(" degrees");
            }
            else
            {
                Serial.println("Invalid Servo 2 angle.");
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
    servo1.begin(servo1_center_raw);
    servo2.begin(servo2_center_raw);

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

    // Apply servo commands
    servo1_raw_angle = servo1_center_raw + servo1_relative_angle;
    servo2_raw_angle = servo2_center_raw + servo2_relative_angle;

    servo1.write(servo1_raw_angle);
    servo2.write(servo2_raw_angle);

    // ======================================================
    // Update OLED every 200 ms
    // ======================================================

    static unsigned long lastDisplayMs = 0;

    if (nowMs - lastDisplayMs >= 200)
    {
        lastDisplayMs = nowMs;

        step4_display.update(
            servo1_relative_angle,
            servo2_relative_angle
        );
    }
}