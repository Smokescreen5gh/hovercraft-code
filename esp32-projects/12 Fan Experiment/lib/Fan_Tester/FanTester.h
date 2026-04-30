#pragma once
#include <Arduino.h>
#include <Wire.h>

class FanTester {
public:
  FanTester(TwoWire& staticBus, TwoWire& venturiBus);

  void begin();
  void zeroSensors();
  void update();

  float getStaticPressurePa() const;
  float getVenturiPressurePa() const;

  float getFlowM3s() const;
  float getCFM() const;
  float getM3H() const;

  float getV1() const;  // chamber velocity, m/s
  float getV2() const;  // throat velocity, m/s

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

  float _zeroOffsetStatic;
  float _zeroOffsetVenturi;

  float _staticPa;
  float _venturiPa;

  float _flowM3s;
  float _cfm;
  float _m3h;

  float _v1;
  float _v2;

  bool _staticOK;
  bool _venturiOK;
};