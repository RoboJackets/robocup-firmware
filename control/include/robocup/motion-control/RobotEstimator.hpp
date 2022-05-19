#pragma once

#include <cstdint>
#include <Eigen/Dense>
#include "rc-fshare/robot_model.hpp"
#include "common.hpp"

/** @class RobotEstimator
 *  This class implements a Kalman Filter in order to estimate a robot's current state.
 *  The estimated state is then used in motion control to assign wheel velocities accordingly
 *
 *  Explanations:
 *  - https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
 *  - https://en.wikipedia.org/wiki/Kalman_filter
 *  - https://www.mathworks.com/videos/series/understanding-kalman-filters.html
 */
class RobotEstimator {
private:
    /**
     * Number of tracked states (number of states in state vector, x_hat)
     *
     * 3 Total:
     * - X and Y: linear velocities of robot body (m/s)
     * - W: angular velocity of robot body around z axis (rad/s)
     */
    static constexpr int numStates = 3;

    /**
     * Number of applied inputs (number of control inputs in control vector, u)
     *
     * 4 total:
     * - Motor duty cycle for each motor (% max), controls average applied voltage
     */
    static constexpr int numInputs = 4;

    /**
     * Number of tracked measurable outputs (number of outputs in output vector, y)
     *
     * 5 total:
     * - 4 wheel motor encoders (rad/s)
     * - Gyro angular velocity (rad/s)
     */
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

private:

    static constexpr float processNoise = 0.05;  /**< State noise from unmodeled influences (model errors/omissions) */
    static constexpr float encoderNoise = 0.04;  /**< Measurement noise in the encoder */
    static constexpr float gyroNoise = 0.005;    /**< Measurement noise in the gyro */
    static constexpr float initCovariance = 0.1; /**< The initial covariance value in each entry of `P` */

    /**
     * State Transition Matrix/Prediction Matrix
     *
     * Predict our next state based on our current state using dynamics
     */
    Eigen::Matrix<float, numStates,  numStates>  F;

    /**
     * Control Input Matrix
     *
     * Corrects our next state prediction by mapping control input (known influences, `u`) to our next state
     */
    Eigen::Matrix<float, numStates,  numInputs>  B;

    /**
     * Observation Matrix
     *
     * Predicts the measurements from our sensors (`z`) from our current state (`x_hat`).
     */
    Eigen::Matrix<float, numOutputs, numStates>  H;

    /**
     * Covariance Matrix for Process Noise
     *
     * A matrix whose entries are the estimated covariances between any two state variables.
     * Entries represent uncertainty in our state vector (`x_hat`) due to unmodeled influences.
     */
    Eigen::Matrix<float, numStates,  numStates>  Q;

    /**
     * Covariance Matrix for Observation Noise
     *
     * A matrix whose entries are the estimated covariances between any two state variables.
     * Entries represent uncertainty in our measurements (`z`) due to random noise
     */
    Eigen::Matrix<float, numOutputs, numOutputs> R;

    /**
     * Covariance Estimate Matrix
     *
     * A matrix whose entries are the estimated covariances between any two state variables.
     * Entries represent our overall uncertainty in our state vector (`x_hat`).
     */
    Eigen::Matrix<float, numStates,  numStates>  P;

    /**
     * Identity Matrix
     *
     * A square matrix with only 1s on the diagonals
     */
    Eigen::Matrix<float, numStates, numStates> I;

    /**
     * Current State Vector Estimate
     *
     * What the robot estimator believes our current state to be
     */
    Eigen::Matrix<float, numStates, 1> x_hat;
};
