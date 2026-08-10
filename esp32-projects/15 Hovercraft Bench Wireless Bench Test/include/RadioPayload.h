#pragma once
#include <Arduino.h>

// What kind of packet is this?
enum class PacketType : uint8_t {
  HEARTBEAT = 0,
  POT      = 1,
  MOTOR_STATUS = 2
};

// This is the payload that gets sent over NRF (must be <= 32 bytes)
struct RadioPayload {
  PacketType type;       // HEARTBEAT or TEXT or Telemetary
  uint16_t   counter;    // increments every send (helps debugging) basically monitors how many times the packets the struct has been sent. can help identify if any packets were gone
  uint16_t   pot1Raw;
  uint16_t   pot2Raw;
  char       motor1State;
  char       motor2State;
};

// NRF24L01 payload max is 32 bytes by default
static_assert(sizeof(RadioPayload) <= 32, "RadioPayload must be <= 32 bytes!");