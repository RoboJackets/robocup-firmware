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
    // todo drop these gains when turning on wheel controllers
    BodyKp << 1, 1.5, 1;
    BodyKi << 0.02, 0.02, 0.04;

    BodyErrorSum << 0, 0, 0;
    BodyILimit << 0.5, 0.5, 2;

    BodyPrevTarget << 0, 0, 0;

    // Wheel
    // Gains should be the same across all wheels for now
    WheelKp << 1, 1, 1, 1;
}

void RobotController::calculateBody(Eigen::Matrix<double, numStates, 1> pv,
                                    Eigen::Matrix<double, numStates, 1> sp,
                                    Eigen::Matrix<double, numWheels, 1>& outputs) {
    // Force accel to be in robot physical limits
    // Assume we can limit linear accel and thats good enough
    // to limit wheel accel correctly to not slip
    BodyInputLimited = limitBodyAccel(sp, dampedTarget);

    // get error
    // todo, replace with sp with damped target
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

    // todo make sure no specific wheel goes over max duty cycle
    // todo replace 511 with duty cycle max
    // clean up clip code
    // remove the last two to output rad/s
    outputs = RobotModel::get().BotToWheel * outputSpeed * RobotModel::get().SpeedToDutyCycle / 511;

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

    /*

    debugInfo.val[8] = dampedTarget(0, 0) * 1000;
    debugInfo.val[9] = dampedTarget(1, 0) * 1000;
    debugInfo.val[10] = dampedTarget(2, 0) * 1000;

    debugInfo.val[11] = pv(0, 0) * 1000;
    debugInfo.val[12] = pv(1, 0) * 1000;
    debugInfo.val[13] = pv(2, 0) * 1000; */

    BodyPrevTarget = dampedTarget;
}

void RobotController::calculateWheel(Eigen::Matrix<double, numWheels, 1> pv,
                                     Eigen::Matrix<double, numWheels, 1> sp,
                                     Eigen::Matrix<double, numWheels, 1>& outputs) {
    return; // disable until we have time to tune
    // todo check for nan?

    Eigen::Matrix<double, numWheels, 1> error = sp - pv;

    outputs = sp + WheelKp.cwiseProduct(error);
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

    /*
    debugInfo.val[0] = pv(0, 0) * 1000;
    debugInfo.val[1] = pv(1, 0) * 1000;
    debugInfo.val[2] = pv(2, 0) * 1000;
    debugInfo.val[3] = pv(3, 0) * 1000;

    debugInfo.val[4] = sp(0, 0) * 1000;
    debugInfo.val[5] = sp(1, 0) * 1000;
    debugInfo.val[6] = sp(2, 0) * 1000;
    debugInfo.val[7] = sp(3, 0) * 1000;
    */
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