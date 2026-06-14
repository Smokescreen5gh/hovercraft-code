#include <Arduino.h>

// ---------- IR Sensor ----------
#define IR_PIN 27

// ---------- RPM Settings ----------
#define PULSES_PER_REV 1

// ---------- Pulse Rejection Settings ----------
#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

// ---------- Pulse Tolerance Settings -----------
#define PERIOD_TOLERANCE 0.1
#define MAX_CONSECUTIVE_OUTLIERS 10

unsigned long lastGoodPeriodUs = 0;
unsigned long acceptedPulseCount = 0;
unsigned long rejectedOutlierCount = 0;
unsigned long consecutiveOutlierCount = 0;

// --------- 0 RPM Settings -------
#define RPM_ZERO_TIMEOUT_MS 2000
float rpm = 0.0;
unsigned long lastAcceptedPulseMs = 0;

// ---- RPM Filter Settings ------
float rpmFiltered = 0.0;
const float filterAlpha = 0.05;

// ---------- Interrupt Variables ----------
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTimeUs = 0;
volatile unsigned long latestPeriodUs = 0;
volatile bool newPulseAvailable = false;
volatile unsigned long rejectedFastPulseCount = 0;

// This function runs automatically when GPIO 27 sees a falling edge
// This is the interupt function
void IRAM_ATTR handlePulse() {
  // Tracks what time of the clock the interrupt got triggered and stores it in the variable nowUs
  unsigned long nowUs = micros();

  // ---------------- Assign time to first pulse ----------------------
  // nowUs is stored in this variable and the period calcuation will start when the 2nd pulse and onwards commence
  if (lastPulseTimeUs == 0) {
    lastPulseTimeUs = nowUs;
    return;
  }

  
  //----------------- Calculate the period ---------------------------
  // After the second pulse, the period can be calculated
  unsigned long dt  = nowUs - lastPulseTimeUs;

  // ----------------- Reject Fake Pulse -----------------------------
  // Reject fake pulses that are too fast to be real
  // Checks if the period dt is fake or not
  if (dt < MIN_PULSE_INTERVAL_US) {
    rejectedFastPulseCount++;
    return;
  }

  // Stores the calculated period
  latestPeriodUs = dt;

  // The tracked pulse will now be considered as the prevoius pulse 
  lastPulseTimeUs = nowUs;
  

  // Increase Pulse Timer
  pulseCount++;
  newPulseAvailable = true;
}

// ----------------- Outlier Rejection Function ---------------
bool isPeriodReasonable(unsigned long newPeriod, unsigned long referencePeriod) {
  if (referencePeriod == 0) {
    return true;
  }

  float lowerLimit = referencePeriod * (1.0 - PERIOD_TOLERANCE);
  float upperLimit = referencePeriod * (1.0 + PERIOD_TOLERANCE);

  return (newPeriod >= lowerLimit && newPeriod <= upperLimit);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IR_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(IR_PIN), handlePulse, FALLING);

  Serial.println("Phase 7: False Pulse Detection");
}

void loop() {
  bool pulseAvailableCopy;
  unsigned long periodCopy;
  unsigned long pulseCountCopy;

  static unsigned long lastSerialMs = 0;
  unsigned long nowMs = millis();

  // Copy interrupt variables safely.
  noInterrupts();
  pulseAvailableCopy = newPulseAvailable;
  periodCopy = latestPeriodUs;
  pulseCountCopy = pulseCount;
  newPulseAvailable = false;
  interrupts();

 // When the loop recieves a pulse from the interrupt
 // We want period copy to be greater than 0 because we cant divide by 0 
  if (pulseAvailableCopy && periodCopy > 0) {
    bool acceptPulse = false; // First deny it as an acceptable pulse, we will run checks if the given pulse is legit or not

    // Accept first few pulses so the code can establish a normal period
    if (acceptedPulseCount < 5) {
      acceptPulse = true;
    } else {
      acceptPulse = isPeriodReasonable(periodCopy, lastGoodPeriodUs); // For every pulse afterwards, we will check if the period is good or not. It takes the current period measurment from the interupt and the last goodperiodmeasurment
    }

    // if the pulse is good, we then tick the counter and calculate the rpm 
    if (acceptPulse) {
      lastGoodPeriodUs = periodCopy;
      acceptedPulseCount++;
      consecutiveOutlierCount = 0;
      lastAcceptedPulseMs = nowMs;

      // --------- Calculate RPM -------------------
      rpm = (60.0 * 1000000.0) / (periodCopy * PULSES_PER_REV);  // Calculate RPM

      // Filtered RPM
      if (rpmFiltered <= 0.0) {
        rpmFiltered = rpm;
        } else {
        rpmFiltered = (filterAlpha * rpm) + ((1.0 - filterAlpha) * rpmFiltered);
        }
    } 

    else {
      rejectedOutlierCount++;
      consecutiveOutlierCount++;

      if (consecutiveOutlierCount >= MAX_CONSECUTIVE_OUTLIERS) {
        lastGoodPeriodUs = 0;
        acceptedPulseCount = 0;
        consecutiveOutlierCount = 0;
      }

    }
  }

  if (lastAcceptedPulseMs > 0 && nowMs - lastAcceptedPulseMs > RPM_ZERO_TIMEOUT_MS) {
    rpm = 0.0;
    rpmFiltered = 0.0;

    lastGoodPeriodUs = 0;
    acceptedPulseCount = 0;
    consecutiveOutlierCount = 0;

    noInterrupts();
    lastPulseTimeUs = 0;
    latestPeriodUs = 0;
    newPulseAvailable = false;
    pulseCount = 0;
    rejectedFastPulseCount = 0;
    interrupts();

    lastAcceptedPulseMs = 0;
  }

  if (nowMs - lastSerialMs >= 50) {
    lastSerialMs = nowMs;

    
    Serial.print("RPM: ");
    Serial.print(rpm, 0);

    Serial.print(" | Accepted: ");
    Serial.print(acceptedPulseCount);

    Serial.print(" | OutlierReject: ");
    Serial.print(rejectedOutlierCount);

    Serial.print(" | FastReject: ");
    Serial.println(rejectedFastPulseCount);
    
    Serial.print(">rpm:");
    Serial.println(rpm);

    Serial.print(">filtered:");
    Serial.println(rpmFiltered);
  }

}