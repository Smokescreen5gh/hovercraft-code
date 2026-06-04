#include "FanTester.h"

// ─── Sensor I2C address ───────────────────────────────────────────────────────
#define SENSOR_ADDR 0x28

// ─── ELVH-L04D transfer-function constants ────────────────────────────────────
// Differential, 10%-90% transfer function (Type A)
// Offset = 8192 counts (zero pressure)
// FSS    = 13108 counts (total span for ±4 inH2O differential)
// Range  = 8 inH2O total = 1990.6 Pa total
const float OFFSET_COUNTS     = 8192.0f;
const float FULL_SCALE_COUNTS = 13108.0f;
const float PRESSURE_RANGE_PA = 1990.6f;

// ─── Air properties ───────────────────────────────────────────────────────────
const float AIR_DENSITY = 1.2f;   // kg/m³ at ~20°C, sea level

// ─── Chamber geometry (A1) ───────────────────────────────────────────────────
const float CHAMBER_WIDTH_M  = 0.2032f;   // 8 in → m
const float CHAMBER_HEIGHT_M = 0.2032f;   // 8 in → m
const float CHAMBER_AREA_M2  = CHAMBER_WIDTH_M * CHAMBER_HEIGHT_M;  // 0.04129 m²

// ─── Throat geometry (A2) ────────────────────────────────────────────────────
const float THROAT_WIDTH_M  = 0.1016f;    // 4 in → m
const float THROAT_HEIGHT_M = 0.1016f;    // 4 in → m
const float THROAT_AREA_M2  = THROAT_WIDTH_M * THROAT_HEIGHT_M;     // 0.01032 m²

// ─── Venturi correction terms (pre-computed) ─────────────────────────────────
// Full venturi equation: Q = A2 * sqrt( 2*dP / (rho * (1 - (A2/A1)^2)) )
// areaRatio  = A2/A1 = 0.25
// betaTerm   = 1 - (0.25)^2 = 0.9375
const float AREA_RATIO = THROAT_AREA_M2 / CHAMBER_AREA_M2;          // 0.25
const float BETA_TERM  = 1.0f - (AREA_RATIO * AREA_RATIO);          // 0.9375

// ─── Discharge coefficient ───────────────────────────────────────────────────
// Cd accounts for real-world losses through the converging nozzle.
// Typical value for a smooth nozzle: 0.95–0.98.
// Tune this against your anemometer: Cd = V_anemometer_avg / V2_calculated
// V_anemometer_avg = 0.85 * V_anemometer_center  (velocity profile correction)
const float Cd = 0.97f;

// ─── EMA filter coefficient ──────────────────────────────────────────────────
// new_ema = ALPHA * raw + (1 - ALPHA) * old_ema
// Lower  = smoother but slower to respond  (0.05 → ~20 sample lag)
// Higher = faster but noisier              (0.3  → ~3  sample lag)
// At 50ms update rate, ALPHA=0.1 gives roughly a 1-second smoothing window —
// similar to the mechanical averaging of a handheld anemometer.
const float EMA_ALPHA = 0.1f;

// ─── Minimum deltaP threshold ────────────────────────────────────────────────
// Below this value treat flow as zero to avoid noise near zero.
const float MIN_DELTA_P = 1.0f;   // Pa

// ─────────────────────────────────────────────────────────────────────────────

FanTester::FanTester(TwoWire& staticBus, TwoWire& venturiBus)
  : _staticBus(staticBus),
    _venturiBus(venturiBus),
    _zeroOffsetStatic(0.0f),
    _zeroOffsetVenturi(0.0f),
    _staticPa(0.0f),
    _venturiPa(0.0f),
    _emaStaticPa(0.0f),
    _emaVenturiPa(0.0f),
    _emaInitialized(false),
    _flowM3s(0.0f),
    _v1(0.0f),
    _v2(0.0f),
    _staticOK(false),
    _venturiOK(false)
{
}

// ─────────────────────────────────────────────────────────────────────────────

void FanTester::begin() {
  zeroSensors();
}

// ─────────────────────────────────────────────────────────────────────────────

void FanTester::zeroSensors() {
  Serial.println("Zeroing pressure sensors — make sure fan is OFF.");

  float accStatic  = 0.0f;
  float accVenturi = 0.0f;
  int goodStatic   = 0;
  int goodVenturi  = 0;

  for (int i = 0; i < 32; i++) {
    uint16_t counts;
    uint8_t  status;

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

  if (goodStatic  > 0) _zeroOffsetStatic  = accStatic  / goodStatic;
  if (goodVenturi > 0) _zeroOffsetVenturi = accVenturi / goodVenturi;

  // Seed the EMA filters with the zero offsets so they don't
  // start from 0.0 and take a long time to settle on startup.
  _emaStaticPa  = 0.0f;
  _emaVenturiPa = 0.0f;
  _emaInitialized = false;

  Serial.print("Static zero offset:  "); Serial.print(_zeroOffsetStatic,  2); Serial.println(" Pa");
  Serial.print("Venturi zero offset: "); Serial.print(_zeroOffsetVenturi, 2); Serial.println(" Pa");
}

// ─────────────────────────────────────────────────────────────────────────────

void FanTester::update() {
  uint16_t countsStatic  = 0;
  uint16_t countsVenturi = 0;
  uint8_t  statusStatic  = 0;
  uint8_t  statusVenturi = 0;

  // 1. Read both sensors
  _staticOK  = readSensor(_staticBus,  countsStatic,  statusStatic);
  _venturiOK = readSensor(_venturiBus, countsVenturi, statusVenturi);

  // 2. Convert to Pa (raw, zero-corrected)
  if (_staticOK) {
    _staticPa = countsToPressurePa(countsStatic, _zeroOffsetStatic);
  }

  if (_venturiOK) {
    _venturiPa = countsToPressurePa(countsVenturi, _zeroOffsetVenturi);
  }

  // 3. Apply EMA filter to both channels
  //    On the very first valid read, seed the EMA directly so it
  //    doesn't have to ramp up from 0.
  if (_staticOK && _venturiOK) {
    if (!_emaInitialized) {
      _emaStaticPa    = _staticPa;
      _emaVenturiPa   = _venturiPa;
      _emaInitialized = true;
    } else {
      _emaStaticPa  = EMA_ALPHA * _staticPa  + (1.0f - EMA_ALPHA) * _emaStaticPa;
      _emaVenturiPa = EMA_ALPHA * _venturiPa + (1.0f - EMA_ALPHA) * _emaVenturiPa;
    }

    // 4. Compute flow and velocities from FILTERED deltaP
    _flowM3s = pressureToFlowM3s(_emaVenturiPa);
    _v1      = flowToV1(_flowM3s);
    _v2      = flowToV2(_flowM3s);

  } else {
    // If either sensor fails, zero everything out
    _emaVenturiPa = 0.0f;
    _emaStaticPa  = 0.0f;
    _flowM3s      = 0.0f;
    _v1           = 0.0f;
    _v2           = 0.0f;
  }
}

// ─────────────────────────────────────────────────────────────────────────────

bool FanTester::readSensor(TwoWire& bus, uint16_t& counts, uint8_t& status) {
  bus.requestFrom((uint8_t)SENSOR_ADDR, (uint8_t)4);

  if (bus.available() != 4) return false;

  uint8_t b1 = bus.read();
  uint8_t b2 = bus.read();
  bus.read();   // temperature MSB  — not used
  bus.read();   // temperature LSB  — not used

  status = (b1 >> 6) & 0x03;
  counts = ((uint16_t)(b1 & 0x3F) << 8) | b2;

  return true;
}

// ─────────────────────────────────────────────────────────────────────────────

float FanTester::countsToPressurePa(uint16_t counts, float offset) {
  return ((float)counts - OFFSET_COUNTS) / FULL_SCALE_COUNTS * PRESSURE_RANGE_PA - offset;
}

// ─────────────────────────────────────────────────────────────────────────────

float FanTester::pressureToFlowM3s(float deltaP) {
  // Reject noise near zero
  if (deltaP <= MIN_DELTA_P) return 0.0f;

  // Full incompressible venturi equation with discharge coefficient:
  // Q = Cd * A2 * sqrt( 2*deltaP / (rho * (1 - (A2/A1)^2)) )
  return Cd * THROAT_AREA_M2 * sqrtf((2.0f * deltaP) / (AIR_DENSITY * BETA_TERM));
}

// ─────────────────────────────────────────────────────────────────────────────

float FanTester::flowToV1(float flowM3s) {
  // Continuity: V1 = Q / A1
  return flowM3s / CHAMBER_AREA_M2;
}

float FanTester::flowToV2(float flowM3s) {
  // Continuity: V2 = Q / A2
  return flowM3s / THROAT_AREA_M2;
}

// ─────────────────────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────────────────────

float FanTester::getStaticPressurePa()  const { return _staticPa;      }
float FanTester::getVenturiPressurePa() const { return _venturiPa;     }
float FanTester::getStaticPaFiltered()  const { return _emaStaticPa;   }
float FanTester::getVenturiPaFiltered() const { return _emaVenturiPa;  }
float FanTester::getFlowM3s()           const { return _flowM3s;       }
float FanTester::getV1()                const { return _v1;            }
float FanTester::getV2()                const { return _v2;            }
bool  FanTester::staticSensorOK()       const { return _staticOK;      }
bool  FanTester::venturiSensorOK()      const { return _venturiOK;     }