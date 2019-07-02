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

RobotController::RobotController(uint32_t dt_us)
    : BodyUseILimit(true), BodyInputLimited(false),
      BodyOutputLimited(false), dt(dt_us/1000000.0) {

    // Body
    BodyKp << 0, 0, 0.5; // 1, 1.5, 1;
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
    Eigen::Matrix<double, numStates, 1> dampedTarget;

    // Force accel to be in robot physical limits
    // Assume we can limit linear accel and thats good enough
    // to limit wheel accel correctly to not slip
    BodyInputLimited = limitBodyAccel(sp, dampedTarget);

    // get error
    Eigen::Matrix<double, numStates, 1> error = dampedTarget - pv;

    // Don't allow the integral to keep going unless we are within a range
    // and not saturated
    // TODO: Stop the problem where output limits reset integral such that it integrates
    // up to max again
    for (int i = 0; i < numStates; i++) {
        if (!BodyInputLimited && !BodyOutputLimited) {
            if (bounded(error(i,0), -BodyILimit(i,0), BodyILimit(i,0))) {
                BodyErrorSum(i,0) += error(i,0);
            } else {
                BodyErrorSum(i,0) = error(i,0);
            }
        }
    }

    // FF + P
    Eigen::Matrix<double, numStates, 1> outputSpeed = dampedTarget +
                                                      BodyKp.cwiseProduct(error) +
                                                      BodyKi.cwiseProduct(BodyErrorSum);

    // todo, clip max vel
    // 3m/s and 20rad/s?
    outputs = RobotModel::get().BotToWheel * outputSpeed;

    debugInfo.val[8] = pv(0, 0) * 100;
    debugInfo.val[9] = pv(1, 0) * 100;
    debugInfo.val[10] = pv(2, 0) * 100;

    debugInfo.val[11] = dampedTarget(0, 0) * 100;
    debugInfo.val[12] = dampedTarget(1, 0) * 100;
    debugInfo.val[13] = dampedTarget(2, 0) * 100;

    BodyPrevTarget = dampedTarget;
}

void RobotController::calculateWheel(Eigen::Matrix<double, numWheels, 1> pv,
                                     Eigen::Matrix<double, numWheels, 1> sp,
                                     Eigen::Matrix<double, numWheels, 1>& outputs) {
    Eigen::Matrix<double, numWheels, 1> dampedTarget;
    limitWheelAccel(sp, dampedTarget);

    Eigen::Matrix<double, numWheels, 1> error = dampedTarget - pv;

    // todo replace 511 with duty cycle max
    // clean up clip code
    outputs = dampedTarget + WheelKp.cwiseProduct(error);
    outputs = outputs * RobotModel::get().SpeedToDutyCycle / 511;

    // todo, calc actual max body vel somewhere else
    for (int i = 0; i < 4; i++) {
        if (outputs(i,0) > 1.0) {
            outputs(i,0) = 1.0;
            BodyOutputLimited = true;
        } else if (outputs(i,0) < -1.0) {
            outputs(i,0) = -1.0;
            BodyOutputLimited = true;
        } else {
            BodyOutputLimited = false;
        }
    }

    debugInfo.val[0] = pv(0, 0) * 100;
    debugInfo.val[1] = pv(1, 0) * 100;
    debugInfo.val[2] = -pv(2, 0) * 100;
    debugInfo.val[3] = -pv(3, 0) * 100;

    debugInfo.val[4] = dampedTarget(0, 0) * 100;
    debugInfo.val[5] = dampedTarget(1, 0) * 100;
    debugInfo.val[6] = -dampedTarget(2, 0) * 100;
    debugInfo.val[7] = -dampedTarget(3, 0) * 100;

    for (int i = 14; i < 18; i++)
    {
        debugInfo.val[i] = outputs(i-14, 0) * 100;
    }

    WheelPrevTarget = dampedTarget;
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
            delta(i, 0) = dt*maxWheelAccel;
            limited = true;
        }

        if (delta(i, 0) < -dt*maxWheelAccel) {
            delta(i, 0) = -dt*maxWheelAccel;
            limited = true;
        }
    }

    dampened = delta + WheelPrevTarget;

    return limited;
}