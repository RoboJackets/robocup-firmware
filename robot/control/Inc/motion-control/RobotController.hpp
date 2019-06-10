#pragma once

#include <cstdint>

class RobotController {
public:
    RobotController(uint32_t dt);

    /**
     * Updates the controller with the latest input and calculates
     * the correct motor commands to reach this target
     * 
     * @param currentState Current state of the robot (XYW velocities)
     * @param setpoint Target to reach (XYW velocity)
     * @param output Next command to send to the motors (duty cycles)
     */
    void calculate(bool currentState, bool setpoint, bool& output);
};