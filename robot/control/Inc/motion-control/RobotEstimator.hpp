#pragma once

#include <cstdint>
#include <Eigen/Dense>

// todo improve documentation on these
class RobotEstimator {
private:
    // X Y W bot velocities (m/s)
    static constexpr int numStates = 3;

    // Motor duty cycles (% max)
    static constexpr int numInputs = 4;

    // Motor encoders (rad/s), gyro angular vel (rad/s)
    static constexpr int numOutputs = 5;

public:
    /**
     * @param dt_us Expected period of the controller in us
     */
    RobotEstimator(uint32_t dt_us);

    /**
     * Using the previous state and the next input
     * We can guess where we are this time step
     *
     * @param u Last motor command
     */
    void predict(Eigen::Matrix<float, numInputs, 1> u);

    /**
     * Using the next measurements, we can move our prediction
     * closer to the true target
     *
     * @param z Encoders 1-4 then gyro
     */
    void update(Eigen::Matrix<float, numOutputs, 1> z);

    /**
     * @param state Matrix that the current guess will be saved into
     */
    void getState(Eigen::Matrix<float, numStates, 1>& state);

    void updateVision(Eigen::Matrix<float, numStates, 1> z); 
private:
    static constexpr float processNoise = 0.05;
    static constexpr float encoderNoise = 0.04;
    static constexpr float gyroNoise = 0.005;
    static constexpr float initCovariance = 0.1;

    Eigen::Matrix<float, numStates,  numStates>  F;
    Eigen::Matrix<float, numStates,  numInputs>  B;
    Eigen::Matrix<float, numOutputs, numStates>  H;

    // Identity
    Eigen::Matrix<float, numStates, numStates> I;

    // Current estimate
    Eigen::Matrix<float, numStates * 2, 1> x_hat;
};
