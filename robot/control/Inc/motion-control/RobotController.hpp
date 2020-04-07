#pragma once

#include <cstdint>
#include <Eigen/Dense>

class RobotController {
private:
    // X Y W bot velocities (m/s)
    const static int numStates = 3;

    // Wheel 1-4 duty cycle % max
    const static int numWheels = 4;

public:
    RobotController(uint32_t dt_us);

    /**
     * Updates the controller with the latest input and calculates
     * the correct wheel velocities to reach this target
     *
     * @param pv Current state (XYW vel in m/s or rad/s)
     * @param sp Current target (XYW vel in m/s or rad/s)
     * @param output Motor targets (rad/s)
     */
    void calculateBody(Eigen::Matrix<float, numStates, 1> pv,
                       Eigen::Matrix<float, numStates, 1> sp,
                       Eigen::Matrix<float, numWheels, 1>& outputs);

    /**
     * Updates the wheels such that they try to follow the target
     * Outputs the correct motor commands to do this
     *
     * @param pv Current state (W1-4 in rad/s)
     * @param sp Current target (W1-4 in rad/s)
     * @param output Motor duty cycles in % max (-1 to 1)
     */
    void calculateWheel(Eigen::Matrix<float, numWheels, 1> pv,
                        Eigen::Matrix<float, numWheels, 1> sp,
                        Eigen::Matrix<float, numWheels, 1>& outputs);

private:
    // Body Vel
    Eigen::Matrix<float, numStates, 1> BodyKp;

    float dt;
};