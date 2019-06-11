#include "motion-control/RobotEstimator.hpp"
#include "rc-fshare/robot_model.hpp"

RobotEstimator::RobotEstimator(uint32_t dt_ms) {
    const double dt = dt_ms/1000.0;

    // Assume constant velocity
    F << 1,  0,  0,
          0, 1,  0,
          0,  0, 1;

    // Assume no input for the moment
    // May need to change based on filter lag
    B << 0, 0, 0, 0,
         0, 0, 0, 0,
         0, 0, 0, 0;

    //H = [bot2Wheel;
    //      0, 0, 1]
    H.block<4, 3>(0, 0) = RobotModel::get().BotToWheel;
    H.block<1, 3>(4, 0) << 0, 0, 1;

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

void RobotEstimator::predict(Eigen::Matrix<double, numInputs, 1> u) {
    // x_hat = F*x_hat + B*last command
    // P = F*P*F' + Q
    x_hat = F*x_hat + B*u;
}

void RobotEstimator::update(Eigen::Matrix<double, numOutputs, 1> z) {
    // y = z - H*x_hat
    // S = H*P*H' + R
    // K = P*H'*S^-1
    // x_hat += K*y
    // P = (I - K*H)*P
    Eigen::Matrix<double, numOutputs, 1> y = z - H*x_hat;
    Eigen::Matrix<double, numOutputs, numOutputs> S = H*P*H.transpose() + R;
    Eigen::Matrix<double, numStates,  numOutputs> K = P*H.transpose()*S.inverse();

    x_hat += K*y;
    P = (I - K*H)*P;
}

void RobotEstimator::getState(Eigen::Matrix<double, numStates, 1>& state) {
    state = x_hat;
}