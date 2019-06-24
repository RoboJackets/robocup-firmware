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
    : useILimit(true), inputLimited(false), outputLimited(false), dt(dt_us/1000000.0) {

    Kp << 1, 1.5, 1;
    Ki << 0.02, 0.02, 0.04;

    errorSum << 0, 0, 0;
    iLimit << 0.5, 0.5, 2;

    prevTarget << 0, 0, 0;
}

void RobotController::calculate(Eigen::Matrix<double, numStates, 1> pv,
                                Eigen::Matrix<double, numStates, 1> sp,
                                Eigen::Matrix<double, numOutputs, 1>& outputs) {
    // Force accel to be in robot physical limits
    // Assume we can limit linear accel and thats good enough
    // to limit wheel accel correctly to not slip
    inputLimited = limitAccel(sp, dampedTarget);

    // get error
    // todo, replace with sp with damped target
    Eigen::Matrix<double, numStates, 1> error = dampedTarget - pv;

    // Don't allow the integral to keep going unless we are within a range
    // and not saturated
    // TODO: Stop the problem where output limits reset integral such that it integrates
    // up to max again
    for (int i = 0; i < numStates; i++) {
        if (!inputLimited && !outputLimited) {
            if (bounded(error(i,0), -iLimit(i,0), iLimit(i,0))) {
                errorSum(i,0) += error(i,0);
            } else {
                errorSum(i,0) = error(i,0);
            }
        }
    }

    // FF + P
    Eigen::Matrix<double, numStates, 1> outputSpeed = dampedTarget + Kp.cwiseProduct(error) + Ki.cwiseProduct(errorSum);

    // todo make sure no specific wheel goes over max duty cycle
    // todo replace 512 with duty cycle max
    // clean up clip code
    outputs = RobotModel::get().BotToWheel * outputSpeed * RobotModel::get().SpeedToDutyCycle / 512;

    for (int i = 0; i < 4; i++) {
        if (outputs(i,0) > 1.0) {
            outputs(i,0) = 1.0;
            outputLimited = true;
        } else if (outputs(i,0) < -1.0) {
            outputs(i,0) = -1.0;
            outputLimited = true;
        } else {
            outputLimited = false;
        }
    }

    prevTarget = dampedTarget;

    debugInfo.val[0] = pv(1, 0) * 1000;
    debugInfo.val[1] = sp(1, 0) * 1000;
    debugInfo.val[3] = dampedTarget(1, 0) * 1000;
}

bool RobotController::limitAccel(const Eigen::Matrix<double, numStates, 1> finalTarget,
                                 Eigen::Matrix<double, numStates, 1>& dampened) {
    Eigen::Matrix<double, numStates, 1> delta = finalTarget - prevTarget;

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

    dampened = delta + prevTarget;

    return limited = true;
}