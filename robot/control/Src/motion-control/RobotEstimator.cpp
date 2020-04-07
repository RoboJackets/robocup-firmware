#include "motion-control/RobotEstimator.hpp"
#include "rc-fshare/robot_model.hpp"
#include "MicroPackets.hpp"

extern DebugInfo debugInfo;

RobotEstimator::RobotEstimator(uint32_t dt_us) {
    const float dt = dt_us/1000000.0;

    // Assume constant velocity
    F << 1,  0,  0,
         0,  1,  0,
         0,  0,  1;

    // Assume no input for the moment
    // May need to change based on filter lag
    B << 0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0;

    //H = [bot2Wheel;
    //      0, 0, 1]
    H.block<4, 3>(0, 0) = RobotModel::get().BotToWheel.cast<float>();
    H.block<1, 3>(4, 0) << 0, 0, 1;

    // Assume no motion
    x_hat << 0, 0, 0;
}

// TODO: Profile these functions
void RobotEstimator::predict(Eigen::Matrix<float, numInputs, 1> u) {
    x_hat = F*x_hat + B*u;
}

void RobotEstimator::update(Eigen::Matrix<float, numOutputs, 1> z) {
    Eigen::Matrix<float, numOutputs, 1> y = z - H*x_hat;
    Eigen::Matrix<float, numStates,  numOutputs> K;

    K <<
        0.001941, -0.002376, -0.002376, 0.001941, -0.001040,
        0.001117, 0.001002, -0.001002, -0.001117, 0.000000,
        -0.004554, -0.004132, -0.004132, -0.004554, 0.152357;

    x_hat += K * y;
    // printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\r\n", z(0), z(1), z(2), z(3), z(4), x_hat(0), x_hat(1), x_hat(2));
    printf("%f\t%f\t%f\r\n", x_hat(0), x_hat(1), x_hat(2));
}

void RobotEstimator::getState(Eigen::Matrix<float, numStates, 1>& state) {
    state = x_hat;
}