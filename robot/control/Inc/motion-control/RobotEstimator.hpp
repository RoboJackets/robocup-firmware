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
    void predict(Eigen::Matrix<double, numInputs, 1> u);

    /**
     * Using the next measurements, we can move our prediction
     * closer to the true target
     * 
     * @param z Encoders 1-4 then gyro
     */
    void update(Eigen::Matrix<double, numOutputs, 1> z);

    /**
     * @param state Matrix that the current guess will be saved into
     */
    void getState(Eigen::Matrix<double, numStates, 1>& state);

private:
    static constexpr double processNoise = 0.01;
    static constexpr double encoderNoise = 0.4;
    static constexpr double gyroNoise = 0.001;
    static constexpr double initCovariance = 10.0;

    Eigen::Matrix<double, numStates,  numStates>  F;
    Eigen::Matrix<double, numStates,  numInputs>  B;
    Eigen::Matrix<double, numOutputs, numStates>  H;
    Eigen::Matrix<double, numStates,  numStates>  Q;
    Eigen::Matrix<double, numOutputs, numOutputs> R;
    Eigen::Matrix<double, numStates,  numStates>  P;

    // Identity
    Eigen::Matrix<double, numStates, numStates> I;

    // Current estimate
    Eigen::Matrix<double, numStates, 1> x_hat;
};