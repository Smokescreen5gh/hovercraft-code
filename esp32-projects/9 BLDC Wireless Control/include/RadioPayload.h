#pragma once
#include <Arduino.h>

// What kind of packet is this?
enum class PacketType : uint8_t {
  HEARTBEAT = 0,
  TEXT      = 1
};

// This is the payload that gets sent over NRF (must be <= 32 bytes)
struct RadioPayload {
  PacketType type;       // HEARTBEAT or TEXT
  uint16_t   counter;    // increments every send (helps debugging) basically monitors how many times the packets the struct has been sent. can help identify if any packets were gone
  char       text[24];   // message text (null-terminated)
};

// NRF24L01 payload max is 32 bytes by default
static_assert(sizeof(RadioPayload) <= 32, "RadioPayload must be <= 32 bytes!");