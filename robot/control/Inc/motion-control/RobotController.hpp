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
    void calculateBody(Eigen::Matrix<double, numStates, 1> pv,
                       Eigen::Matrix<double, numStates, 1> sp,
                       Eigen::Matrix<double, numWheels, 1>& outputs);

    /**
     * Updates the wheels such that they try to follow the target
     * Outputs the correct motor commands to do this
     * 
     * @param pv Current state (W1-4 in rad/s)
     * @param sp Current target (W1-4 in rad/s)
     * @param output Motor duty cycles in % max (-1 to 1)
     */
    void calculateWheel(Eigen::Matrix<double, numWheels, 1> pv,
                         Eigen::Matrix<double, numWheels, 1> sp,
                         Eigen::Matrix<double, numWheels, 1>& outputs);

private:
    /**
     * Limits the difference between the previous target and the new target
     * such that the acceleration limits below are never broken
     */
    bool limitBodyAccel(const Eigen::Matrix<double, numStates, 1> finalTarget,
                        Eigen::Matrix<double, numStates, 1>& dampened);

    bool limitWheelAccel(const Eigen::Matrix<double, numWheels, 1> finalTarget,
                        Eigen::Matrix<double, numWheels, 1>& dampened);


    // Body Vel
    Eigen::Matrix<double, numStates, 1> BodyKp;
    Eigen::Matrix<double, numStates, 1> BodyKi;
    
    Eigen::Matrix<double, numStates, 1> BodyErrorSum;

    // Restrict the i term from contributing more than
    // this amount to the output
    Eigen::Matrix<double, numStates, 1> BodyILimit;
    bool BodyUseILimit;
    bool BodyInputLimited;
    bool BodyOutputLimited;

    Eigen::Matrix<double, numStates, 1> BodyPrevTarget;


    // Wheel Vel
    Eigen::Matrix<double, numWheels, 1> WheelKp;

    Eigen::Matrix<double, numWheels, 1> WheelPrevTarget;

    double dt;

    // Max wheel acceleration rad per second^2)
    static constexpr double maxWheelAccel = 160;//60//120

    // Max acceleration (meters per second^2)
    static constexpr double maxForwardAccel = 8;//4;
    // Max acceleration (meters per second^2)
    static constexpr double maxSideAccel = 8;//2.2;

    // Max angular acceleration (rad per second^2)
    static constexpr double maxAngularAccel = 40.0;
};