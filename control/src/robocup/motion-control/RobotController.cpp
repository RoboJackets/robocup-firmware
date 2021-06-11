#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"
#include <cstdlib>

extern DebugInfo debugInfo;

RobotController::RobotController(uint32_t dt_us) {
    // Proportional constants.
    BodyKp << 0.2, 0.2, 0.05;
}

void RobotController::calculateBody(Eigen::Matrix<float, numStates, 1> pv,
                                    Eigen::Matrix<float, numStates, 1> sp,
                                    Eigen::Matrix<float, numWheels, 1>& outputs) {
    Eigen::Matrix<float, numWheels, numStates> G = RobotModel::get().BotToWheel.cast<float>();
    Eigen::Matrix<float, numStates, 1> error = sp - pv;

    outputs = G * (BodyKp.cwiseProduct(error) + sp);
}

void RobotController::calculateWheel(Eigen::Matrix<float, numWheels, 1> pv,
                                     Eigen::Matrix<float, numWheels, 1> sp,
                                     Eigen::Matrix<float, numWheels, 1>& outputs) {
    outputs = sp / RobotModel::get().SpeedToDutyCycle;
    return;
}
