#pragma once
#include <Arduino.h>

enum class PacketType : uint8_t
{
    HEARTBEAT,
    CONTROL,
    TELEMETRY
};

struct ControlPayload
{
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
};

struct TelemetryPayload
{
    uint8_t Servo_1_Angle;
    uint8_t Servo_2_Angle;
    uint8_t Servo_3_Angle;
    uint8_t Servo_4_Angle;

    char motor_1_state;
    char motor_2_state;
    char motor_3_state;
    char motor_4_state;
    char motor_5_state;
    char motor_6_state;
};


struct RadioPayload
{
    PacketType type;
    uint16_t counter;

    union
    {
        ControlPayload control;
        TelemetryPayload telemetry;
    };
};