V3 8/15/2026
-----------------------------------------------------
Bugs/Improvements
-----------------------------------------------------
=================  Improvmenets ======================
Ammended NRF Driver to use any sort of Payload.h placed in the include folder
Changed NRF_Radio.cpp to NRF_Radio.tpp

This driver is now esssentailly the hardware intializer and states the methods to use on this hardware
In the include section, you can define any struct you want by simply making a new .h file

In the main program, you intialize the radio, and select any of the .h files in the include folder

What this does change allows is that I can now change between different payload files depending on the project. I want this feature so that I can test different payloads or have older payload files

This is similar to:
    - SRC folder
        - I can have differnt programs and just need to intialize them in the ini file. 
        - Don't need to make a new project for every new program
    
    - Lib folder for OLED Data
        - Depending on the program, I can have different data to be displayed on an OLED screen

The NRF radio driver was modified to be payload-agnostic by converting NrfRadio into a templated class. Previously, the driver was hard-coded to use a single RadioPayload structure, which meant every transmitter/receiver configuration had to share and continuously modify the same payload file.

With the new approach, each test configuration can define and use its own payload type:

V2 8/11/2026
-----------------------------------------------------
Bugs/Improvements
-----------------------------------------------------

=================  Improvmenets ======================
Made a small ammendment to the heartbeat method
    - Original driver file had project specific payload data 
        - included hb.Pot1Raw = 0;
        - included hb.Pot2Raw = 0;
        - These were specific to an earlier project

Project Specific Variables should only be handled in RadioPayload.h 

NRF_Radio.cpp and NRF_Radio.h is only responsible for establishing connection, sending the packets, recieving the packets, initalizing the radio, and checking for heartbeat