#include "FanTester.h"

#define SENSOR_ADDR 0x28

const float OFFSET_COUNTS     = 8192.0f;
const float FULL_SCALE_COUNTS = 13108.0f;
const float PRESSURE_RANGE_PA = 1990.6f;

const float AIR_DENSITY     = 1.2f;
const float THROAT_WIDTH_M  = 0.1016f;  // 4 in
const float THROAT_HEIGHT_M = 0.1016f;  // 4 in
const float THROAT_AREA_M2  = THROAT_WIDTH_M * THROAT_HEIGHT_M;

const float M3S_TO_CFM = 2118.88f;
const float M3S_TO_M3H = 3600.0f;

FanTester::FanTester(TwoWire& staticBus, TwoWire& venturiBus)
  : _staticBus(staticBus),
    _venturiBus(venturiBus),
    _zeroOffsetStatic(0.0f),
    _zeroOffsetVenturi(0.0f),
    _staticPa(0.0f),
    _venturiPa(0.0f),
    _cfm(0.0f),
    _m3h(0.0f),
    _staticOK(false),
    _venturiOK(false)
{
}

void FanTester::begin() {
  zeroSensors();
}

void FanTester::zeroSensors() {
  Serial.println("Zeroing pressure sensors. Make sure fan is OFF.");

  float accStatic = 0.0f;
  float accVenturi = 0.0f;
  int goodStatic = 0;
  int goodVenturi = 0;

  for (int i = 0; i < 32; i++) {
    uint16_t counts;
    uint8_t status;

    if (readSensor(_staticBus, counts, status) && status == 0) {
      accStatic += ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA;
      goodStatic++;
    }

    if (readSensor(_venturiBus, counts, status) && status == 0) {
      accVenturi += ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA;
      goodVenturi++;
    }

    delay(20);
  }

  if (goodStatic > 0) {
    _zeroOffsetStatic = accStatic / goodStatic;
  }

  if (goodVenturi > 0) {
    _zeroOffsetVenturi = accVenturi / goodVenturi;
  }

  Serial.print("Static zero offset: ");
  Serial.print(_zeroOffsetStatic, 2);
  Serial.println(" Pa");

  Serial.print("Venturi zero offset: ");
  Serial.print(_zeroOffsetVenturi, 2);
  Serial.println(" Pa");
}

void FanTester::update() {
  uint16_t countsStatic = 0;
  uint16_t countsVenturi = 0;
  uint8_t statusStatic = 0;
  uint8_t statusVenturi = 0;

  _staticOK = readSensor(_staticBus, countsStatic, statusStatic);
  _venturiOK = readSensor(_venturiBus, countsVenturi, statusVenturi);

  if (_staticOK) {
    _staticPa = countsToPressurePa(countsStatic, _zeroOffsetStatic);
  }

  if (_venturiOK) {
    _venturiPa = countsToPressurePa(countsVenturi, _zeroOffsetVenturi);
    _cfm = pressureToFlowCFM(_venturiPa);
    _m3h = pressureToFlowM3h(_venturiPa);
  } else {
    _cfm = 0.0f;
    _m3h = 0.0f;
  }
}

bool FanTester::readSensor(TwoWire& bus, uint16_t& counts, uint8_t& status) {
  bus.requestFrom((uint8_t)SENSOR_ADDR, (uint8_t)4);

  if (bus.available() != 4) {
    return false;
  }

  uint8_t b1 = bus.read();
  uint8_t b2 = bus.read();
  bus.read();
  bus.read();

  status = (b1 >> 6) & 0x03;
  counts = ((uint16_t)(b1 & 0x3F) << 8) | b2;

  return true;
}

float FanTester::countsToPressurePa(uint16_t counts, float offset) {
  return ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA - offset;
}

float FanTester::pressureToVelocity(float deltaP) {
  if (deltaP <= 1.0f) return 0.0f;
  return sqrtf((2.0f * deltaP) / AIR_DENSITY);
}

float FanTester::pressureToFlowM3s(float deltaP) {
  return THROAT_AREA_M2 * pressureToVelocity(deltaP);
}

float FanTester::pressureToFlowCFM(float deltaP) {
  return pressureToFlowM3s(deltaP) * M3S_TO_CFM;
}

float FanTester::pressureToFlowM3h(float deltaP) {
  return pressureToFlowM3s(deltaP) * M3S_TO_M3H;
}

float FanTester::getStaticPressurePa() const {
  return _staticPa;
}

float FanTester::getVenturiPressurePa() const {
  return _venturiPa;
}

float FanTester::getCFM() const {
  return _cfm;
}

float FanTester::getM3H() const {
  return _m3h;
}

bool FanTester::staticSensorOK() const {
  return _staticOK;
}

bool FanTester::venturiSensorOK() const {
  return _venturiOK;
}