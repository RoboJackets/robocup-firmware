#pragma once
#include "RobotModel.hpp"

namespace motion {

class RobotController {
public:
    RobotController();

    // Calculate the controller's values given pose and velocity
    // as well as reference pose, velocity, and acceleration.
    MotorVoltage calculate(
            RobotPose x, RobotTwist v,
            RobotPose rx, RobotTwist rv, RobotTwist ra);

private:
    // Control is done in (x, y, th) coordinates and run through
    // inverse dynamics to get motor voltages.
    Matrix<3, 6> K;

    RobotModel model;
};

}
