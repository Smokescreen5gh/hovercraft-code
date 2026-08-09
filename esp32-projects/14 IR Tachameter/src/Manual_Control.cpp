#include <Arduino.h>
#include <ESP32Servo.h>

#include "DisplayManager.h"
#include "bldc.h"
#include "Tachometer.h"

float rpm = 0;

// --- Desired RPM Setpoint ---
float setpoint = 5000;  // Target RPM (change with "sp 5000")

// --- Throttle Control ---
int currentThrottle = 1000;
int targetThrottle = 1000;
#define MAX_THROTTLE_STEP_US 5

// --- Serial Start/Stop ---
bool systemEnabled = false;
bool escArmed = false;
unsigned long armStartMs = 0;
#define ESC_ARM_TIME_MS 2000

// Display Objects
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C
#define SDA_PIN 21
#define SCL_PIN 22

DisplayManager main_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_I2C_ADDRESS, SDA_PIN, SCL_PIN);

// Motor & Tach
#define motor_1 32
BLDC motor1(motor_1, "M1", 1000, 2000);

#define IR_Pin 27
Tachometer tach1(IR_Pin, 1);

void logEvent(const char* event) {
  Serial.print("#EVENT,");
  Serial.print(millis());
  Serial.print(",");
  Serial.println(event);
}

void checkSerialCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "s") {
      systemEnabled = true;
      escArmed = false;
      armStartMs = millis();
      currentThrottle = 1000;
      targetThrottle = 1000;
      motor1.setThrottle(1000);
      Serial.println("SYSTEM STARTED - ARMING ESC");
      logEvent("SYSTEM_START");
    }

    else if (cmd == "x") {
      systemEnabled = false;
      escArmed = false;
      currentThrottle = 1000;
      targetThrottle = 1000;
      motor1.disable();
      Serial.println("SYSTEM STOPPED");
      logEvent("SYSTEM_STOP");
    }

    // Set throttle
    else if (cmd.startsWith("t ")) {
      int newThrottle = cmd.substring(2).toInt();
      newThrottle = constrain(newThrottle, 1000, 2000);
      targetThrottle = newThrottle;
      Serial.print("Target Throttle: ");
      Serial.print(targetThrottle);
      Serial.println(" µs");
    }

    // Set desired RPM setpoint
    else if (cmd.startsWith("sp ")) {
      setpoint = cmd.substring(3).toFloat();
      Serial.print("Setpoint RPM: ");
      Serial.println(setpoint);
    }

    else {
      Serial.println("Commands: s=START, x=STOP, t <1000-2000>=THROTTLE, sp <rpm>=SETPOINT");
    }
  }
}

int slewLimit(int currentValue, int targetValue, int maxStep) {
  if (targetValue > currentValue + maxStep) return currentValue + maxStep;
  if (targetValue < currentValue - maxStep) return currentValue - maxStep;
  return targetValue;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.setClock(400000);
  main_display.begin("Manual Throttle + RPM");

  tach1.begin();
  motor1.begin();

  systemEnabled = false;
  motor1.disable();

  Serial.println("Commands: s=START, x=STOP, t <1000-2000>=THROTTLE, sp <rpm>=SETPOINT");
  Serial.println("#CSV,millis,system,armed,rpm,setpoint,error,throttle");
}

void loop() {
  checkSerialCommand();

  static unsigned long lastDisplayMs = 0;
  static unsigned long lastSerialMs = 0;
  unsigned long nowMs = millis();

  TachData data = tach1.measure();
  rpm = data.rpmFiltered;

  // Calculate error
  float error = setpoint - rpm;

  // STOPPED MODE
  if (!systemEnabled) {
    motor1.disable();
    currentThrottle = 1000;
    targetThrottle = 1000;
  }

  // ARMING MODE
  else if (!escArmed) {
    currentThrottle = 1000;
    motor1.setThrottle(currentThrottle);

    if (nowMs - armStartMs >= ESC_ARM_TIME_MS) {
      escArmed = true;
      motor1.enable();
      Serial.println("ESC ARMED - READY FOR THROTTLE COMMANDS");
      logEvent("ARMED");
    }
  }

  // RUNNING MODE
  else {
    currentThrottle = slewLimit(currentThrottle, targetThrottle, MAX_THROTTLE_STEP_US);
    motor1.setThrottle(currentThrottle);
  }


    Serial.print("System: ");
    Serial.print(systemEnabled ? "ON" : "OFF");
    Serial.print(" | Armed: ");
    Serial.print(escArmed ? "YES" : "NO");
    Serial.print(" | Setpoint: ");
    Serial.print(setpoint, 0);
    Serial.print(" | RPM: ");
    Serial.print(rpm, 0);
    Serial.print(" | Error: ");
    Serial.print(error, 0);
    Serial.print(" | Throttle: ");
    Serial.print(currentThrottle);
    Serial.print(" / ");
    Serial.println(targetThrottle);

    // --- Teleplot output ---
    Serial.print(">setpoint:");
    Serial.println(setpoint);

    Serial.print(">rpm:");
    Serial.println(rpm);

    Serial.print(">error:");
    Serial.println(error);

    Serial.print(">throttle:");
    Serial.println(currentThrottle);
  

  // OLED update @ 5 Hz
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    Adafruit_SSD1306& display = main_display.getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Manual Throttle");

    display.setCursor(0, 8);
    display.print("State: ");
    display.print(systemEnabled ? (escArmed ? "RUNNING" : "ARMING") : "STOPPED");

    display.setCursor(0, 20);
    display.print("Setpoint: ");
    display.println(setpoint, 0);

    display.setCursor(0, 30);
    display.print("RPM: ");
    display.print(rpm, 0);
    display.print(" Error: ");
    display.println(error, 0);

    display.setCursor(0, 40);
    display.print("Throttle: ");
    display.print(currentThrottle);
    display.print(" / ");
    display.println(targetThrottle);

    display.setCursor(0, 50);
    display.println("s=START x=STOP t=THR sp=RPM");

    display.display();
  }

  delay(50);
}