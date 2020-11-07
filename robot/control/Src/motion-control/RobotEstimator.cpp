#include "motion-control/RobotEstimator.hpp"
#include "rc-fshare/robot_model.hpp"
#include "MicroPackets.hpp"

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
    H << 17.946877243359655, 31.0849032227006, -2.8663890882986363, -22.58867160982906, 27.89468634088194, -2.8663890882986363, -22.588671609829067, -27.89468634088193, -2.8663890882986363, 17.946877243359655, -31.0849032227006, -2.8663890882986363, 0.0, 0.0, 1.0;

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
    K << 0.0009397509971039829, -0.0011827724277488714, -0.0011827724277488686, 0.0009397509971039868, -0.0011276310514651243, 0.00036444308064997926, 0.00032704060073804455, -0.00032704060073804184, -0.00036444308064998137, -1.1132246847540639e-15, -3.972685526745256e-06, -3.7441398088700513e-06, -3.744139808869744e-06, -3.97268552674491e-06, 0.27013068548630365;

    x_hat += K * y;
    static int i = 0;
    if (i++ % 20 == 0) {
        printf("xhat: %f %f %f\r\n", x_hat(0), x_hat(1), x_hat(2));
        printf("z: %f %f %f %f %f\r\n", z(0), z(1), z(2), z(3), z(4));
        printf("\r\n");
    }
}

void RobotEstimator::getState(Eigen::Matrix<float, numStates, 1>& state) {
    state = x_hat;
}
