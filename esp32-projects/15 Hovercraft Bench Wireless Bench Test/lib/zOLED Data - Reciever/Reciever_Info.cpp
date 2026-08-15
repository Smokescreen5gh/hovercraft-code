#include "Reciever_Info.h"

Reciever_Info::Reciever_Info(DisplayManager& displayManager)
    : _displayManager(displayManager)
{
}

void Reciever_Info::update(
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
                int randomRx )
{
    Adafruit_SSD1306& display = _displayManager.getDisplay();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Title
    display.setCursor(0, 0);
    display.print("TX:");
    display.print(RADIO_TX_ADDR);

    display.setCursor(50, 0);
    display.print(" RX:");
    display.print(RADIO_RX_ADDR);

    display.setCursor(109,0);
    display.print(connected ? "CON" : "DIS");

    display.setCursor(0, 16);
    display.print("J1 X:");
    display.print(joy1X);

    display.setCursor(0, 25);
    display.print("J1 Y:");
    display.print(joy1Y);

    display.setCursor(0, 35);
    display.print("J1 B:");
    display.print(joy1Button ? "ON" : "OFF");

    display.setCursor(65, 16);
    display.print("J2 X:");
    display.print(joy2X);

    display.setCursor(65, 25);
    display.print("J2 Y:");
    display.print(joy2Y);

    display.setCursor(65, 35);
    display.print("J2 B:");
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

    display.setCursor(0, 45);
    display.print("S1:");
    display.print(switch1 ? "O" : "F");

    display.setCursor(40, 45);
    display.print("S2:");
    display.print(switch2 ? "O" : "F");

    display.setCursor(80, 45);
    display.print("S3:");
    display.print(switch3 ? "O" : "F");

    display.setCursor(0,55);
    display.print("NUM: ");
    display.print(randomRx);


    display.display();
}