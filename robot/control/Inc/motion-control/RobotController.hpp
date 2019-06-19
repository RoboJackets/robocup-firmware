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
    RobotController(uint32_t dt_ms);

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
     * Limits the difference between our current state and target
     * such that the acceleration limits below are never broken
     */
    void limitAccel(const Eigen::Matrix<double, numStates, 1> currentState,
                    const Eigen::Matrix<double, numStates, 1> finalTarget,
                    Eigen::Matrix<double, numStates, 1>& dampened);

    static constexpr double maxLinearSpeed = 5; // m/s
    static constexpr double minLinearSpeed = -maxLinearSpeed;
    // Time to go from full reverse to full forward in each linear direction
    static constexpr double linearAccelTime = 1; // s
    // Max change in velocity per second
    static constexpr double maxLinearDeltaVel = 
        (maxLinearSpeed - minLinearSpeed) / linearAccelTime;

    static constexpr double maxAngularSpeed = 10; // rad/s
    static constexpr double minAngularSpeed = -maxAngularSpeed;
    // Time to go from full left rotation to full right
    static constexpr double angularAccelTime = 1; // s
    // Max change in angular velocity per second
    static constexpr double maxAngularDeltaVel =
        (maxAngularSpeed - minAngularSpeed) / angularAccelTime;

    double Kp = 1;
    double Ki = 0.01;

    double dt;

    // Target that we are trying to hit
    // Usually same as sp unless the acceleration is too high
    Eigen::Matrix<double, numStates, 1> dampedTarget;

    // Whether each direction is at max speed or not
    // 1 if not saturated, 0 if saturated
    Eigen::Matrix<double, numStates, 1> saturated;
    // Integral component
    Eigen::Matrix<double, numStates, 1> errorSum;
};