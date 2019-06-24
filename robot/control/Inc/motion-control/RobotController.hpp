#pragma once

#include <cstdint>
#include <Eigen/Dense>

class RobotController {
private:
    // X Y W bot velocities (m/s)
    const static int numStates = 3;

    // Wheel 1-4 duty cycle % max
    const static int numOutputs = 4;

public:
    RobotController(uint32_t dt_us);

    /**
     * Updates the controller with the latest input and calculates
     * the correct motor commands to reach this target
     * 
     * @param pv Current state (XYW vel in m/s or rad/s)
     * @param sp Current target (XYW vel in m/s or rad/s)
     * @param output Motor duty cycles in % max (-1 to 1)
     */
    void calculate(Eigen::Matrix<double, numStates, 1> pv,
                   Eigen::Matrix<double, numStates, 1> sp,
                   Eigen::Matrix<double, numOutputs, 1>& outputs);

    // Using the current speed
    // try to move to setpoint speed
private:
    /**
     * Limits the difference between the previous target and the new target
     * such that the acceleration limits below are never broken
     */
    bool limitAccel(const Eigen::Matrix<double, numStates, 1> finalTarget,
                    Eigen::Matrix<double, numStates, 1>& dampened);

    Eigen::Matrix<double, numStates, 1> Kp;
    Eigen::Matrix<double, numStates, 1> Ki;
    
    Eigen::Matrix<double, numStates, 1> errorSum;

    // Restrict the i term from contributing more than
    // this amount to the output
    Eigen::Matrix<double, numStates, 1> iLimit;
    bool useILimit;
    bool inputLimited;
    bool outputLimited;

    Eigen::Matrix<double, numStates, 1> prevTarget;

    double dt;

    // Target that we are trying to hit
    // Usually same as sp unless the acceleration is too high
    Eigen::Matrix<double, numStates, 1> dampedTarget;

    // Max acceleration (meters per second^2)
    static constexpr double maxForwardAccel = 8;//4;
    // Max acceleration (meters per second^2)
    static constexpr double maxSideAccel = 8;//2.2;

    // Max angular acceleration (rad per second^2)
    static constexpr double maxAngularAccel = 40.0;
};