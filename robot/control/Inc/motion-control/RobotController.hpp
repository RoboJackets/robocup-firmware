#pragma once

#include <cstdint>
#include <Eigen/Dense>

/**
 * Controller for wheel velocities
 */
class RobotController {
private:
    /**
     * Number of tracked states (number of states in state vector)
     *
     * X and Y: linear velocities of robot body (m/s)
     * W: angular velocity of robot body around z axis (rad/s)
     */
    const static int numStates = 3;

    /**
     * Number of wheels controlled
     */
    const static int numWheels = 4;

public:
    /**
     * Constructor
     *
     * @param dt_us Expected period of the controller (microseconds)
     */
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
    /**
     * Body velocity control gains (1/s, 1/s, 1/s)
     */
    Eigen::Matrix<float, numStates, 1> BodyKp;

    /**
     * Wheel velocity control gains (duty cycle / speed)
     */
    float WheelKp;

    /**
     * Interval of control calculations (seconds)
     */
    float dt;
};