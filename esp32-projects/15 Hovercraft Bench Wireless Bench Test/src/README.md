V2 8/10/2026
Created a working transmitter project where it can read all input devices

    - 2 analog sticks (with button reading)
    - 3 Potetntiometers
    - 3 Toggle Switches

Best part, all devices are coded as ojects!!!
    - Object Oriented Libraries for the following
        - Potentiomters
        - Oled Screen
        - NRF Modules
        - Analog Sticks
        - BLDC Motors
        - Servos
        - Toggle Switches

Has a dedicated .h and .cpp for that caters to displaying information on the oled screen
    - All you need to do is write the method that parses the variables that the oled screen needs to display
    - Transmitter.h and Transmitter.cpp handles the nitty gritty formatting, text positing, and text displaying


Will work towards sending input data and recieivng telemtry using NRF Module