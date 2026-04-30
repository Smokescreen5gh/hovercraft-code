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
  float getCFM() const;
  float getM3H() const;

  bool staticSensorOK() const;
  bool venturiSensorOK() const;

private:
  bool readSensor(TwoWire& bus, uint16_t& counts, uint8_t& status);
  float countsToPressurePa(uint16_t counts, float offset);
  float pressureToVelocity(float deltaP);
  float pressureToFlowM3s(float deltaP);
  float pressureToFlowCFM(float deltaP);
  float pressureToFlowM3h(float deltaP);

  TwoWire& _staticBus;
  TwoWire& _venturiBus;

  float _zeroOffsetStatic;
  float _zeroOffsetVenturi;

  float _staticPa;
  float _venturiPa;
  float _cfm;
  float _m3h;

  bool _staticOK;
  bool _venturiOK;
};