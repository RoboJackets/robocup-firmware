#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mJackets.hpp"
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

// Hacks because the full "correct" voltages end up creating a
// horrifically oscillatory system.
// TODO(Kyle): Figure out why these aren't just 1.
constexpr float kBackEmfDamping = 0.7;
constexpr float kVoltageDamping = 0.14;

void apply_wheel_force(const Eigen::Matrix<float, 4, 1> force, const Eigen::Matrix<float, 4, 1> speeds, Eigen::Matrix<float, 4, 1>& outputs) {
    for (int i = 0; i < 4; i++) {
        float torque = force(i) * RobotModel::get().WheelRadius / 3.0;
        float voltage = torque * kCurrentPerTorque * kPhaseResistance / 24.0;
        float back_emf = (float) speeds(i) * RobotModel::get().SpeedToDutyCycle / 512.0;
        outputs(i, 0) = kVoltageDamping * voltage + kBackEmfDamping * back_emf;
    }
}

// Bound x by absLimit component-wise, scaling the whole vector to remain
// within the box constraints. Return whether or not it was limited.
template<typename T>
bool boundScaling(T x, T absLimit, T& out) {
    out = x;
    T ratio = x.cwiseQuotient(absLimit);
    T ratioAbs = ratio.cwiseAbs();
    float ratioMax = ratioAbs.maxCoeff();
    if (ratioMax > 1) {
        out /= ratioMax;
        return true;
    } else {
        return false;
    }
}

RobotController::RobotController(uint32_t dt_us)
    : BodyUseILimit(true), BodyInputLimited(false),
      BodyOutputLimited(false), dt(dt_us/1000000.0) {

    // Body
    BodyKp << 0, 0, 0.0; //1, 1.5, 1;
    BodyKi << 0, 0, 0; //0.02, 0.02, 0.02;

    BodyErrorSum << 0, 0, 0;
    BodyILimit << 0.5, 0.5, 2;

    BodyPrevTarget << 0, 0, 0;

    // Wheel
    // Gains should be the same across all wheels for now
    WheelKp << 1.0, 1.0, 1.0, 1.0; //1.5

    WheelPrevTarget << 0, 0, 0, 0;
}

void RobotController::calculateBody(Eigen::Matrix<float, numStates, 1> pv,
                                    Eigen::Matrix<float, numStates, 1> sp,
                                    Eigen::Matrix<float, numWheels, 1>& outputs) {
    // Limit sideways velocity to <= 6m/s
    if (std::abs(sp(0)) > 6.0) {
        sp(0) = std::signbit(sp(0)) * 6.0;
    }

    Eigen::Matrix<float, numWheels, numStates> G = RobotModel::get().BotToWheel.cast<float>();

    // TODO(Kyle): Why do we arbitrarily multiply this by 0.02?
    Eigen::Matrix<float, numStates, 1> linear_accel = (sp - pv) / dt * 0.02;

    // Calculate inverse dynamics
    Eigen::Matrix<float, numStates, 1> robot_force = Eigen::Matrix<float, 3, 1>(kRobotMassX, kRobotMassY, 30 * kRobotMassH * kRobotRadius).cwiseProduct(linear_accel);
    Eigen::Matrix<float, numWheels, 1> wheel_force = G * robot_force;

    apply_wheel_force(wheel_force, G * pv, outputs);

    // Debug variables
    // [0, 3) body acceleration
    debugInfo.val[0] = linear_accel(0,0) * 1000;
    debugInfo.val[1] = linear_accel(1,0) * 1000;
    debugInfo.val[2] = linear_accel(2,0) * 1000;
    // [3, 6) body velocity setpoints
    debugInfo.val[3] = sp(0,0) * 1000;
    debugInfo.val[4] = sp(1,0) * 1000;
    debugInfo.val[5] = sp(2,0) * 1000;
    // [6, 9) body velocity
    debugInfo.val[6] = pv(0,0) * 1000;
    debugInfo.val[7] = pv(1,0) * 1000;
    debugInfo.val[8] = pv(2,0) * 1000;
    // [10, 14) output voltagess
    debugInfo.val[10] = outputs(0, 0) * 1000;
    debugInfo.val[11] = outputs(1, 0) * 1000;
    debugInfo.val[12] = outputs(2, 0) * 1000;
    debugInfo.val[13] = outputs(3, 0) * 1000;
}

void RobotController::calculateWheel(Eigen::Matrix<float, numWheels, 1> pv,
                                     Eigen::Matrix<float, numWheels, 1> sp,
                                     Eigen::Matrix<float, numWheels, 1>& outputs) {
    outputs = sp;
    return;
}

bool RobotController::limitBodyAccel(const Eigen::Matrix<float, numStates, 1> finalTarget,
                                     Eigen::Matrix<float, numStates, 1>& dampened) {
    Eigen::Vector3f accel(maxForwardAccel, maxSideAccel, maxAngularAccel);
    accel *= dt;
    return boundScaling(finalTarget, accel, dampened);
}

bool RobotController::limitWheelAccel(const Eigen::Matrix<float, numWheels, 1> finalTarget,
                                      Eigen::Matrix<float, numWheels, 1>& dampened) {
    Eigen::Vector4f accel(maxWheelAccel, maxWheelAccel, maxWheelAccel, maxWheelAccel);
    accel *= dt;
    return boundScaling(finalTarget, accel, dampened);
}