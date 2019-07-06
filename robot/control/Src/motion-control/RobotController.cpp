#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"
#include <cmath>

extern DebugInfo debugInfo;

/**
 * Edited sign function to always return -1 or 1
 * 
 * Returns 1 if val >= 0, -1 if val < 0
 */
template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

template <typename T> T bounded(T val, T min, T max) {
    return (val < max && val > min);
}

double limit(double val, double maxabs) {
    if (val > maxabs) {
        return maxabs;
    } else if (val < -maxabs) {
        return -maxabs;
    } else {
        return val;
    }
}

double JERK_LIMIT = 10.0;
double TICK_TIME = 5e-3;
double ROBOT_MASS_X = 6.35;
double ROBOT_MASS_Y = 6.35;
double ROBOT_MASS_H = 6.35 * 0.37;
double ROBOT_RADIUS = 0.0794;

void apply_wheel_force(const Eigen::Matrix<double, 4, 1> force, const Eigen::Matrix<double, 4, 1> speeds, Eigen::Matrix<double, 4, 1>& outputs) {
    double CURRENT_PER_TORQUE = 1.0 / 25.1e-3;
    double PHASE_RESISTANCE = 0.464;
    for (int i = 0; i < 4; i++) {
        double torque = force(i) * RobotModel::get().WheelRadius / 3.0;
        double voltage = torque * CURRENT_PER_TORQUE * PHASE_RESISTANCE / 24.0;
        double back_emf = (double) speeds(i) * RobotModel::get().SpeedToDutyCycle / 512.0;
        outputs(i, 0) = 0.14 * voltage + back_emf * 0.7;
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
    if (std::abs(sp(0)) > 6.0) {
        sp(0) = sgn(sp(0)) * 6.0;
    }
    
    Eigen::Matrix<double, numStates, 1> linear_accel = (sp - pv) / dt * 0.02; 


    static Eigen::Matrix<double, numStates, 1> prev_linear_accel(0, 0, 0);

    Eigen::Matrix<double, numStates, 1> linear_diff = linear_accel - prev_linear_accel;

    linear_diff(0, 0) = limit(linear_diff(0, 0), JERK_LIMIT * TICK_TIME);

    linear_diff(1, 0) = limit(linear_diff(1, 0), JERK_LIMIT * TICK_TIME);

    linear_diff(2, 0) = limit(linear_diff(2, 0), JERK_LIMIT * TICK_TIME * 10);

    linear_accel = prev_linear_accel + linear_diff;

    // Eigen::Matrix<double, 4, 1> wheel_accel = RobotModel::get().BotToWheel * linear_accel;
    // for (int i = 0; i < numStates; i++) {
    //     if (fabs(wheel_accel(i, 0)) > 30.0) {
    //         double factor  =fabs(30.0 / wheel_accel(i, 0));
    //         wheel_accel *= factor;
    //         linear_accel *= factor;
    //     }
    // }

    if (std::abs(linear_accel(0)) > 1.5) {
        linear_accel(0) = 1.5 * sgn(linear_accel(0));
    }

    prev_linear_accel = linear_accel;
    linear_accel(2, 0) = sp(2) - pv(2);

    Eigen::Matrix<double, numStates, 1> robot_force = Eigen::Matrix<double, 3, 1>(ROBOT_MASS_X, ROBOT_MASS_Y, 30 * ROBOT_MASS_H * ROBOT_RADIUS).cwiseProduct(linear_accel);

    Eigen::Matrix<double, numWheels, 1> wheel_force = RobotModel::get().BotToWheel * robot_force;

    debugInfo.val[0] = linear_accel(0,0) * 1000;
    debugInfo.val[1] = linear_accel(1,0) * 1000;
    debugInfo.val[2] = linear_accel(2,0) * 1000;
    debugInfo.val[3] = sp(0,0) * 1000;
    debugInfo.val[4] = sp(1,0) * 1000;
    debugInfo.val[5] = sp(2,0) * 1000;
    debugInfo.val[6] = pv(0,0) * 1000;
    debugInfo.val[7] = pv(1,0) * 1000;
    debugInfo.val[8] = pv(2,0) * 1000;
    debugInfo.val[9] = 420 * 1000;
    
    apply_wheel_force(wheel_force, RobotModel::get().BotToWheel * pv, outputs);

    debugInfo.val[10] = outputs(0, 0) * 1000;
    debugInfo.val[11] = outputs(1, 0) * 1000;
    debugInfo.val[12] = outputs(2, 0) * 1000;
    debugInfo.val[13] = outputs(3, 0) * 1000;
}

void RobotController::calculateWheel(Eigen::Matrix<double, numWheels, 1> pv,
                                     Eigen::Matrix<double, numWheels, 1> sp,
                                     Eigen::Matrix<double, numWheels, 1>& outputs) {
    outputs = sp;
    return;
}

bool RobotController::limitBodyAccel(const Eigen::Matrix<double, numStates, 1> finalTarget,
                                     Eigen::Matrix<double, numStates, 1>& dampened) {
    Eigen::Matrix<double, numStates, 1> delta = finalTarget - BodyPrevTarget;

    double accel[numStates] = {maxForwardAccel, maxSideAccel, maxAngularAccel};

    bool limited = false;

    for (int i = 0; i < numStates; i++) {
        // If we broke the accel limit
        if (delta(i, 0) > dt*accel[i]) {
            delta(i,0) = dt*accel[i];
            limited = true;
        }

        if (delta(i, 0) < -dt*accel[i]) {
            delta(i,0) = -dt*accel[i];
            limited = true;
        }
    }

    dampened = delta + BodyPrevTarget;

    return limited;
}

bool RobotController::limitWheelAccel(const Eigen::Matrix<double, numWheels, 1> finalTarget,
                                      Eigen::Matrix<double, numWheels, 1>& dampened) {
    Eigen::Matrix<double, numWheels, 1> delta = finalTarget - WheelPrevTarget;

    bool limited = false;

    for (int i = 0; i < numWheels; i++) {
        if (delta(i, 0) > dt*maxWheelAccel) {
            // delta(i, 0) = dt*maxWheelAccel;
            delta *= dt*maxWheelAccel / delta(i, 0);
            limited = true;
        }

        if (delta(i, 0) < -dt*maxWheelAccel) {
            delta *= -dt*maxWheelAccel / delta(i, 0);
            limited = true;
        }
    }

    dampened = delta + WheelPrevTarget;

    return limited;
}