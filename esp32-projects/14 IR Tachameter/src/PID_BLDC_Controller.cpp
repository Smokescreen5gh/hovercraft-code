#include <Arduino.h>
#include <ESP32Servo.h>

// Custom Libs
#include "DisplayManager.h"
#include "bldc.h"
#include "PID_Display.h"
#include "Tachometer.h"


// -- Some random variables
float rpm = 0;
unsigned long prevT = 0;
float eprev = 0;
float eintegral = 0;

// --- Serial Start/Stop ---
bool systemEnabled = false;
bool escArmed = false;
unsigned long armStartMs = 0;
#define ESC_ARM_TIME_MS 2000

// --- Set Point --------
float setpoint = 3000;

// --- PID Parameters ---
float kp = 0;
float ki = 0;
float kd = 0; 

bool spinupDone = false;
unsigned long spinupStartMs = 0;

#define SPINUP_TIME_MS 5000
#define SPINUP_THROTTLE_US 1150

/*
 --------------------------------------------------------------
       Generate Objects
 --------------------------------------------------------------
*/

// ------- Display Object -----------
// OLED Pin Defintitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

DisplayManager main_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_I2C_ADDRESS, SDA_PIN, SCL_PIN);
PID_Display pidDisplay(main_display);

// ------- BLDC Object -----------
// BLDC Pin Defintion
#define motor_1 32
BLDC motor1(motor_1, "M1", 1000, 2000);

// --------- Tach Object --------
#define IR_Pin 27
Tachometer tach1(IR_Pin,1);

// --------- Serial Command Function --------
void checkSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "s") {
      systemEnabled = true;
      escArmed = false;
      spinupDone = false;
      armStartMs = millis();

      eintegral = 0;
      eprev = 0;
      prevT = micros();

      motor1.setThrottle(1000);

      Serial.println("SYSTEM STARTED - ARMING ESC");
    }

    else if (cmd == "x") {
      systemEnabled = false;
      escArmed = false;
      spinupDone = false;

      eintegral = 0;
      eprev = 0;

      motor1.disable();

      Serial.println("SYSTEM STOPPED");
    }

    else if (cmd.startsWith("sp ")) {
      setpoint = cmd.substring(3).toFloat();
      Serial.print("Setpoint = ");
      Serial.println(setpoint);
    }

    else if (cmd.startsWith("kp ")) {
      kp = cmd.substring(3).toFloat();
      Serial.print("Kp = ");
      Serial.println(kp);
    }

    else if (cmd.startsWith("ki ")) {
      ki = cmd.substring(3).toFloat();
      Serial.print("Ki = ");
      Serial.println(ki);
    }

    else if (cmd.startsWith("kd ")) {
      kd = cmd.substring(3).toFloat();
      Serial.print("Kd = ");
      Serial.println(kd);
    }
  }
}

void setup() {
      Serial.begin(115200);
      delay(200);

      Wire.setClock(400000);
      main_display.begin("RPM Closed Loop Control");



      tach1.begin();
      motor1.begin();

      systemEnabled = false;
      motor1.disable();


      prevT = micros();
      Serial.println("Type s to START, x to STOP");
}

void loop() {
      checkSerialCommand();

      static unsigned long lastDisplayMs = 0;
      unsigned long nowMs = millis();

      TachData data = tach1.measure();
      rpm = data.rpmFiltered;

      int throttleUs = 1000;
      float error = setpoint - rpm;

      // STOPPED MODE
      if (!systemEnabled) {
            motor1.disable();

            eintegral = 0;
            eprev = 0;
            prevT = micros();
      }

      // ARMING MODE
      else if (!escArmed) {
            throttleUs = 1000;
            motor1.setThrottle(throttleUs);

            eintegral = 0;
            eprev = 0;
            prevT = micros();

            if (nowMs - armStartMs >= ESC_ARM_TIME_MS) {
                  escArmed = true;
                  spinupDone = false;
                  spinupStartMs = nowMs;
                  motor1.enable();

                  Serial.println("ESC ARMED - SPINUP");
            }
      }

      // SPINUP MODE
      else if (!spinupDone) {
            throttleUs = SPINUP_THROTTLE_US;
            motor1.setThrottle(throttleUs);

            eintegral = 0;
            eprev = 0;
            prevT = micros();

            if (nowMs - spinupStartMs >= SPINUP_TIME_MS) {
                  spinupDone = true;
                  prevT = micros();

                  Serial.println("SPINUP DONE - PID ACTIVE");
            }
      }

      // PID MODE
      else {
            unsigned long currT = micros();
            float deltaT = (currT - prevT) / 1000000.0;
            prevT = currT;

            float dedt = (error - eprev) / deltaT;

            eintegral = eintegral + error * deltaT;
            eintegral = constrain(eintegral, -15000, 15000);

            float pidOutput =
            kp * error +
            ki * eintegral +
            kd * dedt;

            throttleUs = 1150 + pidOutput;
            throttleUs = constrain(throttleUs, 1000, 2000);

            motor1.setThrottle(throttleUs);

            eprev = error;
      }

      // Serial Print
      // ------- Serial Print -------------
      Serial.print("System: ");
      Serial.print(systemEnabled ? "ON" : "OFF");
      Serial.print(" | Armed: ");
      Serial.print(escArmed ? "YES" : "NO");
      Serial.print(" | Spinup: ");
      Serial.print(spinupDone ? "DONE" : "NO");
      Serial.print(" | Setpoint: ");
      Serial.print(setpoint);
      Serial.print(" | RPM: ");
      Serial.print(rpm);
      Serial.print(" | Error: ");
      Serial.print(error);
      Serial.print(" | Throttle: ");
      Serial.print(throttleUs);
      Serial.print(" | Accepted: ");
      Serial.print(data.acceptedPulseCount);
      Serial.print(" | OutlierReject: ");
      Serial.print(data.rejectedOutlierCount);
      Serial.print(" | FastReject: ");
      Serial.print(data.rejectedFastPulseCount);
      Serial.print(" | Kp: ");
      Serial.print(kp);
      Serial.print(" | Ki: ");
      Serial.print(ki);
      Serial.print(" | Kd: ");
      Serial.print(kd);
      Serial.print(" | State: ");
      Serial.println(motor1.getState());

      // ----- Teleplot -----
      Serial.print(">rpm:");
      Serial.println(rpm);

      Serial.print(">setpoint:");
      Serial.println(setpoint);

      Serial.print(">throttle:");
      Serial.println(throttleUs);

      Serial.print(">error:");
      Serial.println(error);

      // -------------- Display OLED -----------
      if (nowMs - lastDisplayMs >= 200) {
            lastDisplayMs = nowMs;

            pidDisplay.update(
                        systemEnabled,
                        setpoint,
                        rpm,
                        error,
                        throttleUs,
                        kp,
                        ki,
                        kd,
                        motor1.getState()
                  );
      }

      delay(50);
}