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

  // Board 1 -> Board 2
  uint16_t potRaw;
  uint16_t throttleUs;

  // Board 2 -> Board 1
  float staticPa;
  float venturiPa;
  float cfm;
  float v1;
  float v2;

  char motorState;
};

static_assert(sizeof(RadioPayload) <= 32, "RadioPayload must be <= 32 bytes!");