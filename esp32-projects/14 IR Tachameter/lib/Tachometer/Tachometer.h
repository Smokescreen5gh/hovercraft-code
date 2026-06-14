#pragma once

#include <Arduino.h>

// This struct packages all tachometer values together
struct TachData {
  float rpmRaw;
  float rpmFiltered;

  unsigned long acceptedPulseCount;
  unsigned long rejectedOutlierCount;
  unsigned long rejectedFastPulseCount;
};

class Tachometer {
public:
  // Constructor
  Tachometer(int pin, int pulsesPerRev);

  // Sets up pinMode and interrupt
  void begin();

  // Updates tachometer logic and returns RPM data
  TachData measure();

  // ISR needs access to this
  void IRAM_ATTR handlePulse();

private:
  int _pin;
  int _pulsesPerRev;

  // RPM values
  float _rpm = 0.0;
  float _rpmFiltered = 0.0;

  // Outlier rejection variables
  unsigned long _lastGoodPeriodUs = 0;
  unsigned long _acceptedPulseCount = 0;
  unsigned long _rejectedOutlierCount = 0;
  unsigned long _consecutiveOutlierCount = 0;

  // 0 RPM timeout
  unsigned long _lastAcceptedPulseMs = 0;

  // Interrupt variables
  volatile unsigned long _pulseCount = 0;
  volatile unsigned long _lastPulseTimeUs = 0;
  volatile unsigned long _latestPeriodUs = 0;
  volatile bool _newPulseAvailable = false;
  volatile unsigned long _rejectedFastPulseCount = 0;

  // Helper functions
  bool isPeriodReasonable(
    unsigned long newPeriod,
    unsigned long referencePeriod
  );

  void reset();
};