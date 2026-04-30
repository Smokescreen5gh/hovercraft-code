#include "FanTester.h"

#define SENSOR_ADDR 0x28

// ELVH sensor transfer-function constants
const float OFFSET_COUNTS     = 8192.0f;
const float FULL_SCALE_COUNTS = 13108.0f;
const float PRESSURE_RANGE_PA = 1990.6f;

// Air property
const float AIR_DENSITY = 1.2f;  // kg/m^3

// Chamber geometry: A1
const float CHAMBER_WIDTH_M  = 0.2032f;  // 8 in
const float CHAMBER_HEIGHT_M = 0.2032f;  // 8 in
const float CHAMBER_AREA_M2  = CHAMBER_WIDTH_M * CHAMBER_HEIGHT_M;

// Throat geometry: A2
const float THROAT_WIDTH_M  = 0.1016f;  // 4 in
const float THROAT_HEIGHT_M = 0.1016f;  // 4 in
const float THROAT_AREA_M2  = THROAT_WIDTH_M * THROAT_HEIGHT_M;

// Unit conversions
const float M3S_TO_CFM = 2118.88f;
const float M3S_TO_M3H = 3600.0f;

FanTester::FanTester(TwoWire& staticBus, TwoWire& venturiBus)
  : _staticBus(staticBus),
    _venturiBus(venturiBus),
    _zeroOffsetStatic(0.0f),
    _zeroOffsetVenturi(0.0f),
    _staticPa(0.0f),
    _venturiPa(0.0f),
    _flowM3s(0.0f),
    _cfm(0.0f),
    _m3h(0.0f),
    _v1(0.0f),
    _v2(0.0f),
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

  // 1. Read both pressure sensors
  _staticOK = readSensor(_staticBus, countsStatic, statusStatic);
  _venturiOK = readSensor(_venturiBus, countsVenturi, statusVenturi);

  // 2. Convert static sensor counts to chamber static pressure
  if (_staticOK) {
    _staticPa = countsToPressurePa(countsStatic, _zeroOffsetStatic);
  }

  // 3. Convert venturi sensor counts to deltaP, then calculate flow and velocity
  if (_venturiOK) {
    _venturiPa = countsToPressurePa(countsVenturi, _zeroOffsetVenturi);

    // deltaP = chamber pressure - throat pressure
    // First solve for volumetric flow rate Q in m^3/s
    _flowM3s = pressureToFlowM3s(_venturiPa);

    // Then use continuity: Q = A1*v1 = A2*v2
    _v1 = flowToV1(_flowM3s);
    _v2 = flowToV2(_flowM3s);

    // Convert Q into display/comparison units
    _cfm = _flowM3s * M3S_TO_CFM;
    _m3h = _flowM3s * M3S_TO_M3H;
  } else {
    _venturiPa = 0.0f;
    _flowM3s = 0.0f;
    _v1 = 0.0f;
    _v2 = 0.0f;
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

  // Temperature bytes are currently ignored
  bus.read();
  bus.read();

  // Top 2 bits are sensor status
  status = (b1 >> 6) & 0x03;

  // Lower 14 bits are pressure counts
  counts = ((uint16_t)(b1 & 0x3F) << 8) | b2;

  return true;
}

float FanTester::countsToPressurePa(uint16_t counts, float offset) {
  return ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA - offset;
}

float FanTester::pressureToFlowM3s(float deltaP) {
  if (deltaP <= 1.0f) {
    return 0.0f;
  }

  // Full incompressible venturi equation:
  // Q = A2 * sqrt( 2*deltaP / (rho * (1 - (A2/A1)^2)) )
  float areaRatio = THROAT_AREA_M2 / CHAMBER_AREA_M2;
  float correction = 1.0f - (areaRatio * areaRatio);

  if (correction <= 0.0f) {
    return 0.0f;
  }

  return THROAT_AREA_M2 * sqrtf((2.0f * deltaP) / (AIR_DENSITY * correction));
}

float FanTester::flowToV1(float flowM3s) {
  return flowM3s / CHAMBER_AREA_M2;
}

float FanTester::flowToV2(float flowM3s) {
  return flowM3s / THROAT_AREA_M2;
}

float FanTester::getStaticPressurePa() const {
  return _staticPa;
}

float FanTester::getVenturiPressurePa() const {
  return _venturiPa;
}

float FanTester::getFlowM3s() const {
  return _flowM3s;
}

float FanTester::getCFM() const {
  return _cfm;
}

float FanTester::getM3H() const {
  return _m3h;
}

float FanTester::getV1() const {
  return _v1;
}

float FanTester::getV2() const {
  return _v2;
}

bool FanTester::staticSensorOK() const {
  return _staticOK;
}

bool FanTester::venturiSensorOK() const {
  return _venturiOK;
}