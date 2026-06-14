#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// ---------- Pulse Detection Variables ----------
int previousIRState = HIGH;
unsigned long pulseCount = 0;

// ---------- Timing Variables ----------
unsigned long lastPulseTimeUs = 0;
unsigned long periodUs = 0;

// ---------- RPM Variable ----------
float rpm = 0.0;

// ---------- Teleplot Timing ----------
unsigned long lastTeleplotMs = 0;
const unsigned long TELEPLOT_INTERVAL_MS = 10;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  previousIRState = digitalRead(IR_PIN);

  Serial.println("Phase 4.5: Manual RPM + Teleplot Started");
  Serial.println("Teleplot format uses >name:value");
}

void loop() {
  int currentIRState = digitalRead(IR_PIN);

  // This will become 1 only for one loop cycle when a pulse happens
  int pulseEvent = 0;

  // Detect falling edge: previous was HIGH, current is LOW
  if (previousIRState == HIGH && currentIRState == LOW) {
    pulseCount++;
    pulseEvent = 1;

    unsigned long nowUs = micros();

    if (lastPulseTimeUs == 0) {
      Serial.println("First pulse detected. Waiting for next pulse...");
    } else {
      periodUs = nowUs - lastPulseTimeUs;

      rpm = (60.0 * 1000000.0) / (periodUs * PULSES_PER_REV);
    }

    // Save this pulse time for the next period calculation
    lastPulseTimeUs = nowUs;
  }

  previousIRState = currentIRState;

  // Send data to Teleplot at a controlled rate
  unsigned long nowMs = millis();

  if (nowMs - lastTeleplotMs >= TELEPLOT_INTERVAL_MS) {
    lastTeleplotMs = nowMs;

    //Serial Print
    Serial.print("Pulse count = ");
    Serial.print(pulseCount);

    Serial.print(" | Period = ");
    Serial.print(periodUs);
    Serial.print(" us");

    Serial.print(" | RPM = ");
    Serial.println(rpm, 0);

    // Serial Plot
    // Sensor state: HIGH = 1, LOW = 0
    Serial.print(">irState:");
    Serial.println(currentIRState == HIGH ? 1 : 0);

    // Pulse event: normally 0, briefly 1 when falling edge is detected
    Serial.print(">pulseEvent:");
    Serial.println(pulseEvent);

    // Period in microseconds
    Serial.print(">periodUs:");
    Serial.println(periodUs);

    // RPM
    Serial.print(">rpm:");
    Serial.println(rpm);
  }
}