#include "Transmitter_Info.h"


Transmitter_Info::Transmitter_Info(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void Transmitter_Info::update(
                const char* RADIO_TX_ADDR,
                const char* RADIO_RX_ADDR,
                int joy1X, 
                int joy1Y,
                bool joy1Button,
                int joy2X,
                int joy2Y,
                bool joy2Button,
                uint16_t pot1,
                uint16_t pot2,
                uint16_t pot3,
                bool switch1,
                bool switch2,
                bool switch3,
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

    display.setCursor(0, 16);
    display.print("1X:");
    display.print(joy1X);

    display.setCursor(0, 25);
    display.print("1Y:");
    display.print(joy1Y);

    display.setCursor(30, 0);
    display.print("1B:");
    display.print(joy1Button ? "ON" : "OFF");

    display.setCursor(65, 16);
    display.print("2X:");
    display.print(joy2X);

    display.setCursor(65, 25);
    display.print("2Y:");
    display.print(joy2Y);

    display.setCursor(80, 0);
    display.print("2B:");
    display.print(joy2Button ? "ON" : "OFF");

    display.setCursor(0, 8);
    display.print("P1:");
    display.print(pot1);

    display.setCursor(43, 8);
    display.print("P2:");
    display.print(pot2);

    display.setCursor(85, 8);
    display.print("P3:");
    display.print(pot3);

    display.setCursor(0, 35);
    display.print("S1:");
    display.print(switch1 ? "O" : "F");

    display.setCursor(40, 35);
    display.print("S2:");
    display.print(switch2 ? "O" : "F");

    display.setCursor(80, 35);
    display.print("S3:");
    display.print(switch3 ? "O" : "F");

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