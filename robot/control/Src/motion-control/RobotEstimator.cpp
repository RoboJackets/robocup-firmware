#include "motion-control/RobotEstimator.hpp"
#include "rc-fshare/robot_model.hpp"
#include "MicroPackets.hpp"

extern DebugInfo debugInfo;

RobotEstimator::RobotEstimator(uint32_t dt_us) {
    const float dt = dt_us/1000000.0;

    // Assume constant velocity
    F << 0.998,  0,  0,
         0,  0.995,  0,
         -0.001,  0,  0.998;

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

void RobotEstimator::predict(Eigen::Matrix<float, numInputs, 1> u) {
    x_hat.topRows<3>() = F * x_hat.topRows<3>() + B*u;

    float angle = x_hat(5);
    Eigen::Matrix<float, 3, 3> R;
    R <<
        std::cos(angle), -std::sin(angle), 0,
        std::sin(angle), std::cos(angle), 0,
        0, 0, 1;
    x_hat.bottomRows<3>() += 0.005 * R * x_hat.topRows<3>();
}

void RobotEstimator::update(Eigen::Matrix<float, numOutputs, 1> z) {
    Eigen::Matrix<float, numOutputs, 1> y = z - H*x_hat;
    Eigen::Matrix<float, numStates,  numOutputs> K;

    K <<
        0.001941, -0.002376, -0.002376, 0.001941, -0.001040,
        0.001117, 0.001002, -0.001002, -0.001117, 0.000000,
        -0.004554, -0.004132, -0.004132, -0.004554, 0.152357;

    x_hat.topRows<3>() += K * y;
}

void RobotEstimator::updateVision(Eigen::Matrix<float, numStates, 1> z) {
    // Approximate the x_hat from when the frame was actually taken using a
    // fixed latency.
    double latency = 0.07;
    Eigen::Matrix<float, numStates, 1> old_x_hat =
        x_hat.bottomRows<3>() - x_hat.topRows<3>() * latency;

    // TODO(Kyle): Pick non-fudged Kalman gains here.
    Eigen::Matrix<float, numOutputs, 1> y = z - old_x_hat;
    Eigen::Matrix<float, numStates * 2,  numStates> K;
    K.leftRows<3>() = Eigen::Matrix<float, 3, 3>::Identity() * 0.15;
    K.rightRows<3>() = Eigen::Matrix<float, 3, 3>::Identity() * 0.08;

    x_hat.topRows += K * y;
}

void RobotEstimator::getState(Eigen::Matrix<float, numStates, 1>& state) {
    state = x_hat;
}