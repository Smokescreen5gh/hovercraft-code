#include "Reciever_Info.h"

Reciever_Info::Reciever_Info(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void Reciever_Info::update(
                const char* RADIO_TX_ADDR,
                const char* RADIO_RX_ADDR,
                int Servo_1, 
                int Servo_2,
                int Servo_3,
                int Servo_4,
                uint16_t pot1,
                uint16_t pot2,
                uint16_t pot3,
                bool connected,
                char motor1_State,
                char motor2_State,
                char motor3_State,
                char motor4_State,
                char motor5_State,
                char motor6_State)
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(0, 0);
    display.print(connected ? "CON" : "DIS");

    display.setCursor(0, 8);
    display.print("P1:");
    display.print(pot1);

    display.setCursor(43, 8);
    display.print("P2:");
    display.print(pot2);

    display.setCursor(85, 8);
    display.print("P3:");
    display.print(pot3);

    display.setCursor(0, 16);
    display.print("S1:");
    display.print(Servo_1);

    display.setCursor(30, 16);
    display.print("S2");
    display.print(Servo_2);

    display.setCursor(60, 16);
    display.print("S3");
    display.print(Servo_3);

    display.setCursor(90, 16);
    display.print("S4");
    display.print(Servo_4);

    display.setCursor(0, 45);
    display.print("M1:");
    display.print(motor1_State);

    display.setCursor(60, 45);
    display.print("M2:");
    display.print(motor2_State);

    display.setCursor(00, 55);
    display.print("M3:");
    display.print(motor3_State);

    display.setCursor(30, 55);
    display.print("M4:");
    display.print(motor4_State);

    display.setCursor(60, 55);
    display.print("M5:");
    display.print(motor5_State);

    display.setCursor(100, 55);
    display.print("M6:");
    display.print(motor6_State);

    display.display();
}