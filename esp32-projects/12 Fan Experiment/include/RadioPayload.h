#pragma once
#include <Arduino.h>

enum class PacketType : uint8_t {
  HEARTBEAT = 0,
  CONTROL   = 1,
  TELEMETRY = 2
};

struct RadioPayload {

  PacketType type;
  uint16_t counter;

  // TX → RX (controller → fan tester)
  uint16_t potRaw;
  uint16_t throttleUs;

  // RX → TX (fan tester → controller)
  float staticPa;
  float venturiPa;
  float flowM3s;
  float v1;
  float v2;

  char motorState;
};

static_assert(sizeof(RadioPayload) <= 32,
              "RadioPayload must be <= 32 bytes!");