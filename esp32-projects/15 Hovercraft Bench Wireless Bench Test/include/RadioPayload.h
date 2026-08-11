#pragma once
#include <Arduino.h>

enum class PacketType : uint8_t
{
    HEARTBEAT,
    CONTROL,
    TELEMETRY
};

struct RadioPayload
{
    PacketType type;

    // Generic tracking
    uint16_t counter;

    // -------- CONTROL DATA --------
    uint16_t joy1X;
    uint16_t joy1Y;
    bool joy1Button;

    uint16_t joy2X;
    uint16_t joy2Y;
    bool joy2Button;

    uint16_t pot1;
    uint16_t pot2;
    uint16_t pot3;

    bool switch1;
    bool switch2;
    bool switch3;

    // -------- TELEMETRY DATA --------
    uint8_t randomNumber;
};