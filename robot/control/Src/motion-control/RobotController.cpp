#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"

/**
 * Edited sign function to always return -1 or 1
 * 
 * Returns 1 if val >= 0, -1 if val < 0
 */
template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

RobotController::RobotController(uint32_t dt_ms)
    : dt(dt_ms/1000.0) {

    saturated << 1, 1, 1;
    errorSum << 0, 0, 0;
}

void RobotController::calculate(Eigen::Matrix<double, numStates, 1> pv,
                                Eigen::Matrix<double, numStates, 1> sp,
                                Eigen::Matrix<double, numOutputs, 1>& outputs) {
    // Force accel to be in robot physical limits
    // Assume we can limit linear accel and thats good enough
    // to limit wheel accel correctly to not slip
    limitAccel(pv, sp, dampedTarget);

    // get error
    Eigen::Matrix<double, numStates, 1> error = dampedTarget - pv;

    // Add to the sum only if not saturated
    // todo explore windup
    errorSum += dt*saturated.cwiseProduct(error);

    // FF + PI
    Eigen::Matrix<double, numStates, 1> outputSpeed = dampedTarget + Kp*error + Ki*errorSum;

    // Note: If num states ever changes, this must be changed as well
    double maxSpeeds[numStates] = {maxLinearSpeed, maxLinearSpeed, maxAngularSpeed};

    // Disable integral on next iteration if we are above the max
    // Also limit speeds back into the max speed range
    // TODO: See how the 1 frame lag affects performance
    // TODO: See if need to check signs to add to integral if error in other direction
    for (int i = 0; i < 3; i++) {
        // Push speed back into real range
        // Disable the integral on that channel if saturated
        if (abs(outputSpeed(i, 0)) >= maxSpeeds[i]) {
            outputSpeed(i, 0) = sgn(outputSpeed(i, 0)) * maxSpeeds[i];
            saturated(i, 0) = 0;
        } else {
            saturated(i, 0) = 1;
        }
    }

    outputs = RobotModel::get().BotToWheel * outputSpeed;
}

void RobotController::limitAccel(const Eigen::Matrix<double, numStates, 1> currentState,
                                 const Eigen::Matrix<double, numStates, 1> finalTarget,
                                 Eigen::Matrix<double, numStates, 1>& dampened) {
    Eigen::Matrix<double, numStates, 1> delta = finalTarget - currentState;

    // Note: If num states ever changes, this must be changed as well
    double deltaV[numStates] = {maxLinearDeltaVel, maxLinearDeltaVel, maxAngularDeltaVel};

    for (int i = 0; i < numStates; i++) {
        // If we broke the accel limit
        // just dampen that axis
        // We can assume the path planner gives us a valid path
        if (abs(delta(i, 0)) > dt*deltaV[i]) {
            dampened(i, 0) = currentState(i, 0) + sgn(delta(i, 0))*dt*deltaV[i];
        } else {
            // If we good, just use the final target
            dampened(i, 0) = finalTarget(i, 0);
        }
    }
}