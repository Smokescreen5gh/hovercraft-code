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
        