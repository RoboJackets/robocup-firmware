#pragma once

#include <cstdint>

class RobotEstimator {
public:
    RobotEstimator(uint32_t dt);

    /**
     * Using the previous state and the next intput
     * We can guess where we are this time step
     * 
     * @param lastCommand Last motor command
     */
    void predict(bool lastCommand);

    /**
     * Using the next measurements, we can move our prediction
     * closer to the true target
     * 
     * @param encoders Encoder measurements in rad/sec
     * @param imu IMU measurements in m/s^2 and rad/s
     */
    void update(bool encoders, bool imu);

    /**
     * Gets the current best guess of the state of the robot
     * 
     * @param vector to save the state into
     */
    void getState(bool& state);
};