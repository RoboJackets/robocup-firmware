#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"
#include <cmath>

extern DebugInfo debugInfo;

constexpr double kJerkLimit = 10.0;
constexpr double kTickTime = 5e-3;
constexpr double kRobotMassX = 6.35;
constexpr double kRobotMassY = 6.35;
constexpr double kRobotMassH = 6.35 * 0.37;
constexpr double kRobotRadius = 0.0794;
constexpr double kCurrentPerTorque = 1.0 / 25.1e-3;
constexpr double kPhaseResistance = 0.464;

// Hacks because the full "correct" voltages end up creating a
// horrifically oscillatory system.
// TODO(Kyle): Figure out why these aren't just 1.
constexpr double kBackEmfDamping = 0.7;
constexpr double kVoltageDamping = 0.14;

void apply_wheel_force(const Eigen::Matrix<double, 4, 1> force, const Eigen::Matrix<double, 4, 1> speeds, Eigen::Matrix<double, 4, 1>& outputs) {
    for (int i = 0; i < 4; i++) {
        double torque = force(i) * RobotModel::get().WheelRadius / 3.0;
        double voltage = torque * kCurrentPerTorque * kPhaseResistance / 24.0;
        double back_emf = (double) speeds(i) * RobotModel::get().SpeedToDutyCycle / 512.0;
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
    double ratioMax = ratioAbs.maxCoeff();
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

void RobotController::calculateBody(Eigen::Matrix<double, numStates, 1> pv,
                                    Eigen::Matrix<double, numStates, 1> sp,
                                    Eigen::Matrix<double, numWheels, 1>& outputs) {
    // Limit sideways velocity to <= 6m/s
    if (std::abs(sp(0)) > 6.0) {
        sp(0) = std::signbit(sp(0)) * 6.0;
    }

    // TODO(Kyle): Why do we arbitrarily multiply this by 0.02?
    Eigen::Matrix<double, numStates, 1> linear_accel = (sp - pv) / dt * 0.02;

    static Eigen::Matrix<double, numStates, 1> prev_linear_accel(0, 0, 0);

    Eigen::Matrix<double, numStates, 1> linear_diff = linear_accel - prev_linear_accel;

    // Clamp the acceleration difference (jerk)
    linear_diff(0, 0) = std::clamp(linear_diff(0, 0),
                                   -kJerkLimit * kTickTime,
                                   kJerkLimit * kTickTime);
    linear_diff(1, 0) = std::clamp(linear_diff(1, 0),
                                   -kJerkLimit * kTickTime,
                                   kJerkLimit * kTickTime);
    linear_diff(2, 0) = std::clamp(linear_diff(2, 0),
                                   -kJerkLimit * kTickTime * 10,
                                   kJerkLimit * kTickTime * 10);

    linear_accel = prev_linear_accel + linear_diff;

    // Sideways acceleration is _really_ bad. Limit it.
    constexpr double kMaxSidewaysAccel = 1.5;
    if (std::abs(linear_accel(0)) > kMaxSidewaysAccel) {
        linear_accel(0) = kMaxSidewaysAccel * std::signbit(linear_accel(0));
    }

    prev_linear_accel = linear_accel;
    linear_accel(2, 0) = sp(2) - pv(2);

    // Calculate inverse dynamics
    Eigen::Matrix<double, numStates, 1> robot_force = Eigen::Matrix<double, 3, 1>(kRobotMassX, kRobotMassY, 30 * kRobotMassH * kRobotRadius).cwiseProduct(linear_accel);
    Eigen::Matrix<double, numWheels, 1> wheel_force = RobotModel::get().BotToWheel * robot_force;

    /* debugInfo.val[0] = linear_accel(0,0) * 1000; */
    /* debugInfo.val[1] = linear_accel(1,0) * 1000; */
    /* debugInfo.val[2] = linear_accel(2,0) * 1000; */
    /* debugInfo.val[3] = sp(0,0) * 1000; */
    /* debugInfo.val[4] = sp(1,0) * 1000; */
    /* debugInfo.val[5] = sp(2,0) * 1000; */
    /* debugInfo.val[6] = pv(0,0) * 1000; */
    /* debugInfo.val[7] = pv(1,0) * 1000; */
    /* debugInfo.val[8] = pv(2,0) * 1000; */

    // apply_wheel_force(wheel_force, RobotModel::get().BotToWheel * pv, outputs);
    Eigen::Matrix<double, numWheels, 1> wheel_speed = RobotModel::get().BotToWheel * sp;
    for (int i = 0; i < 4; i++) {
        double sign = 0;
        if (wheel_speed(i) > 1e-3) {
            sign = 1;
        } else if (wheel_speed(i) < -1e-3) {
            sign = -1;
        }
        outputs[i] = wheel_speed(i);
        debugInfo.val[4+i] = wheel_speed(i) * 100;
    }

    debugInfo.val[0] = outputs(0, 0) * 1000;
    debugInfo.val[1] = outputs(1, 0) * 1000;
    debugInfo.val[2] = outputs(2, 0) * 1000;
    debugInfo.val[3] = outputs(3, 0) * 1000;
}

void RobotController::calculateWheel(Eigen::Matrix<double, numWheels, 1> pv,
                                     Eigen::Matrix<double, numWheels, 1> sp,
                                     Eigen::Matrix<double, numWheels, 1>& outputs) {
    for (int i = 0; i < numWheels; i++) {
        double sign = 0;
        if (sp(i) > 1e-3) {
            sign = 1;
        } else if (sp(i) < -1e-3) {
            sign = -1;
        }
        outputs(i) = (sp(i) * RobotModel::get().SpeedToDutyCycle + 50.0 * sign) / 512.0 + 0.01 * (sp(i) - pv(i));
    }
    return;
}

bool RobotController::limitBodyAccel(const Eigen::Matrix<double, numStates, 1> finalTarget,
                                     Eigen::Matrix<double, numStates, 1>& dampened) {
    Eigen::Vector3d accel(maxForwardAccel, maxSideAccel, maxAngularAccel);
    accel *= dt;
    return boundScaling(finalTarget, accel, dampened);
}

bool RobotController::limitWheelAccel(const Eigen::Matrix<double, numWheels, 1> finalTarget,
                                      Eigen::Matrix<double, numWheels, 1>& dampened) {
    Eigen::Vector4d accel(maxWheelAccel, maxWheelAccel, maxWheelAccel, maxWheelAccel);
    accel *= dt;
    return boundScaling(finalTarget, accel, dampened);
}