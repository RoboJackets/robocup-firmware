#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"
#include <cmath>

extern DebugInfo debugInfo;

constexpr float kJerkLimit = 10.0;
constexpr float kTickTime = 5e-3;
constexpr float kRobotMassX = 6.35;
constexpr float kRobotMassY = 6.35;
constexpr float kRobotMassH = 6.35 * 0.37;
constexpr float kRobotRadius = 0.0794;
constexpr float kCurrentPerTorque = 1.0 / 25.1e-3;
constexpr float kPhaseResistance = 0.464;

RobotController::RobotController(uint32_t dt_us) {
    // Body
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