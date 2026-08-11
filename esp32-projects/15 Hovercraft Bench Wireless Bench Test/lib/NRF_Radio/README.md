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