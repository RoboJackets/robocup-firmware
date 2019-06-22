#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"

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

RobotController::RobotController(uint32_t dt_ms)
    : useILimit(true), inputLimited(false), outputLimited(false), dt(dt_ms/1000.0) {

    Kp << 1.5, 2, 1.5;
    Ki << 0.02, 0.02, 0;

    errorSum << 0, 0, 0;
    iLimit << 0.5, 0.5, 0.5;
}

void RobotController::calculate(Eigen::Matrix<double, numStates, 1> pv,
                                Eigen::Matrix<double, numStates, 1> sp,
                                Eigen::Matrix<double, numOutputs, 1>& outputs) {
    // Force accel to be in robot physical limits
    // Assume we can limit linear accel and thats good enough
    // to limit wheel accel correctly to not slip
    limitAccel(pv, sp, dampedTarget);

    // get error
    // todo, replace with sp with damped target
    Eigen::Matrix<double, numStates, 1> error = sp - pv;

    // Don't allow the integral to keep going if we limit
    // either side
    for (int i = 0; i < numStates; i++) {
        if (bounded(error(i,0), -iLimit(i,0), iLimit(i,0))) {
            errorSum(i,0) += error(i,0);
        } else {
            errorSum(i,0) = error(i,0);
        }
    }

    // FF + P
    Eigen::Matrix<double, numStates, 1> outputSpeed = sp + Kp.cwiseProduct(error) + Ki.cwiseProduct(errorSum);

    // todo make sure no specific wheel goes over max duty cycle
    // todo replace 512 with duty cycle max
    // clean up clip code
    outputs = RobotModel::get().BotToWheel * outputSpeed * RobotModel::get().SpeedToDutyCycle / 512;

    debugInfo.val[0] = pv(0, 0) * 1000;
    debugInfo.val[1] = pv(1, 0) * 1000;
    debugInfo.val[2] = pv(2, 0) * 1000;
    debugInfo.val[3] = sp(1, 0) * 1000;

    /*
    static Eigen::Matrix<double, 4, 1> current = outputs;

    for (int i = 0; i < 4; i++) {
        // limit to X% duty cycle change per frame
        double limit = 0.025;
        if (outputs(i,0) - current(i, 0) > limit) {
            outputs(i,0) = current(i, 0) + limit;
        }

        if (current(i,0) - outputs(i, 0) > limit) {
            outputs(i,0) = current(i, 0) - limit;
        }

        if (outputs(i,0) > 1.0f) {
            outputs(i,0) = 1.0f;
        } else if (outputs(i,0) < -1.0f) {
            outputs(i,0) = -1.0f;
        }
    }
    current = outputs;
    */
}

void RobotController::limitAccel(const Eigen::Matrix<double, numStates, 1> currentState,
                                 const Eigen::Matrix<double, numStates, 1> finalTarget,
                                 Eigen::Matrix<double, numStates, 1>& dampened) {
    Eigen::Matrix<double, numStates, 1> delta = finalTarget - currentState;

    // Note: If num states ever changes, this must be changed as well
    double deltaV[numStates] = {maxLinearDeltaVel, maxLinearDeltaVel, maxAngularDeltaVel};

    double percentPastMax = 1.0;

    for (int i = 0; i < numStates; i++) {
        // If we broke the accel limit
        // Scale everything back
        if (abs(delta(i, 0)) > dt*deltaV[i]) {
            percentPastMax = abs(delta(i, 0)) / (dt*deltaV[i]);
        }
    }

    dampened = (1.0 / percentPastMax) * delta + currentState;
}