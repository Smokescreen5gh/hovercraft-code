#pragma once
#include <Arduino.h>
#include <Wire.h>

class FanTester {
public:
  FanTester(TwoWire& staticBus, TwoWire& venturiBus);

  void begin();
  void zeroSensors();
  void update();

  float getStaticPressurePa() const;   // raw static pressure (gauge)
  float getVenturiPressurePa() const;  // raw venturi deltaP

  float getStaticPaFiltered() const;   // EMA-filtered static pressure
  float getVenturiPaFiltered() const;  // EMA-filtered venturi deltaP

  float getFlowM3s() const;            // volumetric flow rate (filtered)
  float getV1() const;                 // chamber velocity m/s (filtered)
  float getV2() const;                 // throat velocity m/s (filtered)

  bool staticSensorOK() const;
  bool venturiSensorOK() const;

private:
  bool readSensor(TwoWire& bus, uint16_t& counts, uint8_t& status);
  float countsToPressurePa(uint16_t counts, float offset);
  float pressureToFlowM3s(float deltaP);
  float flowToV1(float flowM3s);
  float flowToV2(float flowM3s);

  TwoWire& _staticBus;
  TwoWire& _venturiBus;

  // Zero offsets captured at startup
  float _zeroOffsetStatic;
  float _zeroOffsetVenturi;

  // Raw sensor readings
  float _staticPa;
  float _venturiPa;

  // EMA-filtered sensor readings
  float _emaStaticPa;
  float _emaVenturiPa;
  bool  _emaInitialized;   // false until first real reading seeds the EMA

  // Derived outputs (all computed from filtered values)
  float _flowM3s;
  float _v1;
  float _v2;

  bool _staticOK;
  bool _venturiOK;
};