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
    H.block<4, 3>(0, 0) = RobotModel::get().BotToWheel;
    H.block<1, 3>(4, 0) << 0, 0, 0;

    Q.setIdentity();
    Q *= 3.0*processNoise / (dt*dt);

    R.setIdentity();
    R.block<4, 4>(0, 0) *= encoderNoise;
    R.block<1, 1>(4, 4) *= gyroNoise;

    P.setIdentity();
    P *= initCovariance;

    I.setIdentity();

    // Assume no motion
    x_hat << 0, 0, 0;
}

// TODO: Profile these functions
void RobotEstimator::predict(Eigen::Matrix<float, numInputs, 1> u) {
    x_hat = F*x_hat + B*u;
    P = F*P*F.transpose() + Q;
}

void RobotEstimator::update(Eigen::Matrix<float, numOutputs, 1> z) {
    // y = z - H*x_hat
    // S = H*P*H' + R
    // K = P*H'*S^-1
    // x_hat += K*y
    // P = (I - K*H)*P
    Eigen::Matrix<float, numOutputs, 1> y = z - H*x_hat;
    Eigen::Matrix<float, numStates,  numOutputs> K;
    z(4) = x_hat(2);
    K << 1.83624980e-01, -2.29806179e-01, -2.29806179e-01, 1.83624980e-01, -2.13457449e-04,
         3.07632598e-01, 2.76060529e-01, -2.76060529e-01, -3.07632598e-01, -2.99584147e-18,
         -6.86033585e-02, -6.02908809e-02, -6.02908809e-02, -6.86033585e-02, 2.38627208e-02;

    x_hat += K * 0.01 * y;
}

void RobotEstimator::getState(Eigen::Matrix<float, numStates, 1>& state) {
    state = x_hat;
}