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
    Eigen::Matrix<float, numOutputs, 1> zhat = H*x_hat;
    Eigen::Matrix<float, numOutputs, 1> y = z - zhat;
    Eigen::Matrix<float, numStates,  numOutputs> K;
    K << 1.83624980e-01, -2.29806179e-01, -2.29806179e-01, 1.83624980e-01, -2.13457449e-04,
         3.07632598e-01, 2.76060529e-01, -2.76060529e-01, -3.07632598e-01, -2.99584147e-18,
         -6.86033585e-01, -6.02908809e-01, -6.02908809e-01, -6.86033585e-01, 2.38627208e-01;

    x_hat += K * y;
    static int i = 0;
    if (i++ % 20 == 0) {
        printf("xhat: %f %f %f\r\n", x_hat(0), x_hat(1), x_hat(2));
        printf("zhat: %f %f %f %f %f\r\n", zhat(0), zhat(1), zhat(2), zhat(3), zhat(4));
        printf("z: %f %f %f %f %f\r\n", z(0), z(1), z(2), z(3), z(4));
        printf("y: %f %f %f %f %f\r\n", y(0), y(1), y(2), y(3), y(4));
        printf("\r\n");
    }
}

void RobotEstimator::getState(Eigen::Matrix<float, numStates, 1>& state) {
    state = x_hat;
}
