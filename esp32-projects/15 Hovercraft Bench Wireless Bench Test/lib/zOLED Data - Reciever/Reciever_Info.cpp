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
                bool connected)
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
    display.print("S1:");
    display.print(Servo_1);

    display.setCursor(0, 25);
    display.print("S2");
    display.print(Servo_2);

    display.setCursor(0, 35);
    display.print("S3");
    display.print(Servo_3);

    display.setCursor(0, 45);
    display.print("S4");
    display.print(Servo_4);

    display.setCursor(0, 8);
    display.print("Pot1");
    display.print(pot1);

    display.display();
}