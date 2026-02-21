#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// --------------------------- OLED Configuration ---------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1              // Most I2C SSD1306 modules don't use a reset pin
#define OLED_I2C_ADDRESS 0x3C      // Common addresses: 0x3C or 0x3D

// --------------------------- Pin Definitions ---------------------------
// I2C pins for ESP32 (common defaults: SDA=21, SCL=22)
#define SDA_PIN 21
#define SCL_PIN 22

// Onboard LED (varies by board; many devkits use GPIO 2)
#define LED_PIN 2

// Potentiometer ADC pins
#define POT1_PIN 26
#define POT2_PIN 27

// Servo pin (pick a PWM-capable safe GPIO)
#define SERVO_PIN 25

// --------------------------- Global Objects ---------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo myServo;

// --------------------------- Task: LED Blinker ---------------------------
/**
 * TaskLEDBlinker
 * Blinks LED 
 *  - ON  3000ms
 *  - OFF 100ms
 *  - ON  100ms
 *  - OFF 100ms
 *
 * Uses vTaskDelay() so it does NOT block other tasks.
 */
void TaskLEDBlinker(void *pvParameters) {
  (void)pvParameters;

  pinMode(LED_PIN, OUTPUT);

  while (true) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(3000));

    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));

    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));

    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// --------------------------- Task: Pots + Serial + OLED + Servo ---------------------------
/**
 * TaskPotsAndDisplay
 * Reads two potentiometers and prints + displays them at a fixed update rate.
 * Keeping all OLED calls in this one task avoids I2C concurrency issues.
 */
void TaskPotsAndDisplay(void *pvParameters) {
  (void)pvParameters;

  // ADC configuration (ESP32)
  analogReadResolution(12);        // 12-bit ADC -> 0..4095
  analogSetAttenuation(ADC_11db);  // Best for ~0..3.3V input range

  // Servo init (do it inside the task so it happens on this core/task context)
  myServo.setPeriodHertz(50);            // 50Hz servo
  myServo.attach(SERVO_PIN, 500, 2500);  // pulse widths in microseconds



  while (true) {
    // Read potentiometers (raw ADC counts)
    int pot1 = analogRead(POT1_PIN);
    int pot2 = analogRead(POT2_PIN);

    static int lastAngle = -999;

    int angle = map(pot1, 0, 4095, 0, 180);
    angle = constrain(angle, 0, 180);

    if (abs(angle - lastAngle) >= 2) {   // 2° deadband
      myServo.write(angle);
      lastAngle = angle;
    }

    // ---- Serial Monitor Output ----
    Serial.print("POT 1: ");
    Serial.print(pot1);
    Serial.print(" | POT 2: ");
    Serial.print(pot2);
    Serial.print(" | SERVO: ");
    Serial.println(angle);

    // ---- OLED Output ----
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("POT 1:");
    display.println(pot1);

    display.setCursor(67, 0);
    display.print("POT 2:");
    display.println(pot2);

    display.display();

    // Update rate: 20ms -> ~50Hz (controller-like)
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// --------------------------- Arduino Setup ---------------------------
void setup() {
  Serial.begin(115200);
  delay(200); // small startup delay for serial monitor attachment

  // I2C init
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  Serial.println("Initializing OLED...");
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("SSD1306 not found. Check wiring and I2C address (0x3C/0x3D).");
    while (true) { delay(1000); } // halt if OLED init fails
  }

  // Basic OLED settings
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Startup message (shown once)
  display.setCursor(10, 20);
  display.println("Pot Reader Ready");
  display.display();

  Serial.println("OLED initialized");

  // --------------------------- Create RTOS Tasks ---------------------------
  // Task priorities:
  //  - Pots/OLED task priority 2: keep UI/input responsive
  //  - LED task priority 1: lower priority is fine
  //
  // Stack sizes:
  //  - LED: small stack
  //  - OLED task: larger (display libs + buffers)
  //
  // Core pinning (ESP32 has 2 cores):
  //  - LED pinned to core 0
  //  - Pots/OLED pinned to core 1
  // Pinning is optional, but useful for learning and predictability.
 
  // Runs Task 1
  xTaskCreatePinnedToCore(
    TaskLEDBlinker,   // task function
    "LED Blinker",    // name
    2048,             // stack size
    NULL,             // parameters
    1,                // priority
    NULL,             // task handle
    0                 // core (0 is fine)
  );

  // Run Task 2
  xTaskCreatePinnedToCore(
    TaskPotsAndDisplay,   // task function
    "Pots+OLED",          // name
    4096,                 // stack size (OLED needs more)
    NULL,                 // parameters
    2,                    // priority (higher than LED)
    NULL,                 // task handle
    1                     // core (1 is fine)
  );
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

 
