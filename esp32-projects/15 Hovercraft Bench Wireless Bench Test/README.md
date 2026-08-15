V4 8/15/2026
Integrated PCA9685 board to control various PWM devices on the reciever side.

Transmitter has the same input devices and now can control the pwm devices on the reciever side 


-----------------------------------------------------
Bugs/Improvements
-----------------------------------------------------
================= BUGS ===============================



=================  Improvmenets ======================
1. Made payload package agnostic to the NRF Driver 

The NRF radio driver was modified to be payload-agnostic by converting NrfRadio into a templated class. Previously, the driver was hard-coded to use a single RadioPayload structure, which meant every transmitter/receiver configuration had to share and continuously modify the same payload file.

With the new approach, each test configuration can define and use its own payload type:


V3 8/11/2026
Full Bi Directional Transmitter built and tested with a dummy reciever

1. Transmitter actively reads pot data, joystick data, toggle switch data
2. Transmitter relays input data over NRF and sends it to the reciever
3. Transmitter receieves incoming data from dummy reciever (random number generator)
4. Transmitter displays input data and recieved data on OLED

Basically adding on from V2 but now sending data and recieving data via NRF for bidirectional communication


-----------------------------------------------------
Bugs/Improvements
-----------------------------------------------------
================= BUGS ===============================


=================  Improvmenets ======================
First attempt, the reciever had oscillitoary behavior where it will switch between a recieved input value from the transmtter and a value of 0

The issue lied in the way the code was structured not an issue with the connection

Essentially what happened was we did not have any way of storing the incoming data from the control packet. The reciever was just simply pulling out the incoming input values from RadioPayload in{};

The reciever was reading values directly from 
    RadioPaylod in{};

    - Because "in" is declared inside the loop function, a new zero-initalized packet is created on every loop iteration. 
    - - Basically this packet will always intialize back to 0 as it gets recreated every loop
    - We are sending different type of packets: heatbeat, control, telemetry
    - only when a control packet arrives, do the "in" variables gets filled with the input commands

********* Fix ********
Create a place to store the incoming control packet
    - Created RadioPayload controlRx{};

    Whenever a valid control packet arrives
        - controlRx = in
        - we copy that incoming packet "in" to a storage packet "controlRx"
        - our variables will be read from this storage packet instead until the next control packet is written in this 

    Instead of doing: 
        in.joy1X
        in.pot1
        in.switch1

    We do:
        controlRx.joy1X
        controlRx.pot1
        controlRx.switch1

    So everytime we have an incoming packet with the type control, it copies itself into the RadioPayload controlRx{}:
        - So now we pull our data from this stored packet (RadioPayload controlRx{}) instead of (RadioPayload in{}) 

In project 13 "reciever.cpp" I did something similar except I copied each incoming value to a individual vairable like
    static uint16_t pot1Rx;
    static uint16_t pot2Rx;

    Instead of typing each variable to store the value, I decided to copy the entire incoming control packet instead

    That is why I typed
        int throttle1 = map(pot1Rx, 0, 4095, 1000, 2000);
    
    Instead of 
        int throttle1 = map(in.pot1Rx, 0, 4095, 1000, 2000);


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