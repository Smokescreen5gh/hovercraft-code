V3 8/8/2026
Updates:
    - Changes start spinup throttle from 1100 to 1400 (This is where the D3530 spins around 6000 rpm)
        - Since I am testing at 7000 and 8000 rpm, it is easier to get to this set point from higher a throttle
        - Orignally at 11000 rpm, the d3530 motor would spin to like 3000 rpm, when wanting to reach 7000 rpm, the system would have a big overshoot and settling time to reach 7000 rpm

    - So the motor first gradually ramps to 1400 us throttle, spins at that throttle for 5 seconds (spinup complete) then enables PID

    - Changed the tachometer to sense rising edges instead of falling edges.
        - Refelective white tape was not working properly and had to switch to light color impellers and use black electrical tape. 
        - Tachometer senses when black tape is presenet 

V2 6/20/2026

Updates:

    - PID Gains found for 3000 rpm

    - Will need to test for other RPMS 

    - Has a built in function to run a python logger to log PWM data across run time for plotting. Function is commented out for now but can be used when needed
Issues:

    - Tachometer does not work with a power supply. Will resort to using a lipo battery 
    
    - Need to test PID for other RPMs


V1 6/14/2026

Updates:

    - Implemented a semi working PID BLDC Controller

    - Can talk to serial monitor on to enable and disable the system

    - upon enabling, system takes 3 seconds to arm the motor, 5 seconds to spin the motor at a minimal throttle to allow the tachometer to read a steady clean value

    - upon a few tests, the system was able to accuratly track the setpoint speed

Issues:

    - Tachometer spikes to a large rpm value when triggered at first. Needs some time to settle to a proper RPM reading

    - System wants to surge the BLDC when changing speeds or a PID constant. Does not occur when using a Lipo Battery

      - Look into slowly scewing the response to not surge and ramp the response than having a fast response.

        - either decrease kp (need to look into this)

        - implement a ramp sysytem limiting how fast the bldc speed can change 

    - System oscillates at steady state

        - look into changing ki or kd 

        - implement a deadband system
        