#include "Tachometer.h"

// ---------- Pulse Rejection Settings ----------
#define MAX_VALID_RPM 15000
#define MIN_PULSE_INTERVAL_US (60000000UL / MAX_VALID_RPM)

// ---------- Pulse Tolerance Settings ----------
#define PERIOD_TOLERANCE 0.1
#define MAX_CONSECUTIVE_OUTLIERS 10

// ---------- 0 RPM Settings ----------
#define RPM_ZERO_TIMEOUT_MS 2000

// ---------- RPM Filter Settings ----------
const float FILTER_ALPHA = 0.05;

// ---------- Static Interrupt Object ----------
// This lets the interrupt call the Tachometer object
static Tachometer* activeTach = nullptr;

// This function runs automatically when the IR pin sees a falling edge
void IRAM_ATTR tachISR() {
  if (activeTach != nullptr) {
    activeTach->handlePulse();
  }
}

// ---------- Constructor ----------
// This assigns the sensor pin and pulses per revolution
Tachometer::Tachometer(int pin, int pulsesPerRev) {
  _pin = pin;
  _pulsesPerRev = pulsesPerRev;
}

// ---------- Begin Function ----------
// This replaces the tachometer setup code
void Tachometer::begin() {
  pinMode(_pin, INPUT);

  activeTach = this;

  attachInterrupt(
    digitalPinToInterrupt(_pin),
    tachISR,
    FALLING
  );
}

// ---------- Interrupt Function ----------
// This function runs automatically when the sensor sees reflective tape
void IRAM_ATTR Tachometer::handlePulse() {
  // Tracks what time the interrupt got triggered
  unsigned long nowUs = micros();

  // Assign time to first pulse
  // Period calculation starts on the second pulse and onwards
  if (_lastPulseTimeUs == 0) {
    _lastPulseTimeUs = nowUs;
    return;
  }

  // Calculate the period between current pulse and previous pulse
  unsigned long dt = nowUs - _lastPulseTimeUs;

  // Reject fake pulses that are too fast to be real
  if (dt < MIN_PULSE_INTERVAL_US) {
    _rejectedFastPulseCount++;
    return;
  }

  // Store the calculated period
  _latestPeriodUs = dt;

  // The current pulse now becomes the previous pulse
  _lastPulseTimeUs = nowUs;

  // Increase pulse counter
  _pulseCount++;

  // Tell the main loop that a new pulse is ready
  _newPulseAvailable = true;
}

// ---------- Measure Function ----------
// This replaces the tachometer loop code
TachData Tachometer::measure() {
  bool pulseAvailableCopy;
  unsigned long periodCopy;

  unsigned long nowMs = millis();

  // Copy interrupt variables safely
  noInterrupts();
  pulseAvailableCopy = _newPulseAvailable;
  periodCopy = _latestPeriodUs;
  _newPulseAvailable = false;
  interrupts();

  // When the loop receives a pulse from the interrupt
  // periodCopy must be greater than 0 because we cannot divide by 0
  if (pulseAvailableCopy && periodCopy > 0) {
    bool acceptPulse = false;

    // Accept first few pulses so the code can establish a normal period
    if (_acceptedPulseCount < 5) {
      acceptPulse = true;
    } else {
      // For every pulse afterwards, check if the period is reasonable
      // It compares the current period measurement to the last good period
      acceptPulse = isPeriodReasonable(periodCopy, _lastGoodPeriodUs);
    }

    // If the pulse is good, calculate RPM
    if (acceptPulse) {
      _lastGoodPeriodUs = periodCopy;
      _acceptedPulseCount++;
      _consecutiveOutlierCount = 0;
      _lastAcceptedPulseMs = nowMs;

      // Calculate raw RPM
      _rpm = (60.0 * 1000000.0) / (periodCopy * _pulsesPerRev);

      // Calculate filtered RPM
      if (_rpmFiltered <= 0.0) {
        _rpmFiltered = _rpm;
      } else {
        _rpmFiltered =
          (FILTER_ALPHA * _rpm) +
          ((1.0 - FILTER_ALPHA) * _rpmFiltered);
      }
    }

    // If the pulse is bad, reject it as an outlier
    else {
      _rejectedOutlierCount++;
      _consecutiveOutlierCount++;

      // If too many outliers happen in a row, reset lock
      // This lets the tachometer learn a new RPM
      if (_consecutiveOutlierCount >= MAX_CONSECUTIVE_OUTLIERS) {
        _lastGoodPeriodUs = 0;
        _acceptedPulseCount = 0;
        _consecutiveOutlierCount = 0;
      }
    }
  }

  // If no accepted pulse happens for too long, RPM becomes 0
  if (_lastAcceptedPulseMs > 0 &&
      nowMs - _lastAcceptedPulseMs > RPM_ZERO_TIMEOUT_MS) {
    reset();
  }

  // Package tachometer data and return it to main.cpp
  TachData data;
  data.rpmRaw = _rpm;
  data.rpmFiltered = _rpmFiltered;
  data.acceptedPulseCount = _acceptedPulseCount;
  data.rejectedOutlierCount = _rejectedOutlierCount;
  data.rejectedFastPulseCount = _rejectedFastPulseCount;

  return data;
}

// ---------- Outlier Rejection Function ----------
bool Tachometer::isPeriodReasonable(
  unsigned long newPeriod,
  unsigned long referencePeriod
) {
  if (referencePeriod == 0) {
    return true;
  }

  float lowerLimit = referencePeriod * (1.0 - PERIOD_TOLERANCE);
  float upperLimit = referencePeriod * (1.0 + PERIOD_TOLERANCE);

  return newPeriod >= lowerLimit && newPeriod <= upperLimit;
}

// ---------- Reset Function ----------
// This resets RPM if the motor stops or no pulse is detected
void Tachometer::reset() {
  _rpm = 0.0;
  _rpmFiltered = 0.0;

  _lastGoodPeriodUs = 0;
  _acceptedPulseCount = 0;
  _consecutiveOutlierCount = 0;

  noInterrupts();
  _lastPulseTimeUs = 0;
  _latestPeriodUs = 0;
  _newPulseAvailable = false;
  _pulseCount = 0;
  _rejectedFastPulseCount = 0;
  interrupts();

  _lastAcceptedPulseMs = 0;
}