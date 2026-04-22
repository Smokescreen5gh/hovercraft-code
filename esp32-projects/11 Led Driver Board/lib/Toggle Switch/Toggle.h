#pragma once
#include <Arduino.h>

class ToggleSwitch {
    public:

        ToggleSwitch(uint8_t pin, const char* name);

        // Method 1: 
        void begin();

        // Method 2:
        void update();
        
        // Method 3:
        bool isOn() const;
        
    private:
        uint8_t _pin;
        const char* _name;
        bool _status;

};