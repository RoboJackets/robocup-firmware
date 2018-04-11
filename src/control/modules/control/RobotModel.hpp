#pragma once

#define EIGEN_HAS_CXX11_MATH 0
#include <Eigen/Dense>
#include <array>
#include <cmath>
//#include "const-math.hpp"

constexpr double DegreesToRadians(double val) { return val * M_PI / 180.0; }

/// Model parameters for a robot.  Used by the controls system.
class RobotModel {
public:
    /// Radius of omni-wheel (in meters)
    double WheelRadius;

    /// Distance from center of robot to center of wheel
    double WheelDist;

    /// Wheel angles (in radians) measured between +x axis and wheel axle
    std::array<double, 4> WheelAngles;

    /// wheelSpeeds = BotToWheel * V_bot
    Eigen::Matrix<double, 4, 3> BotToWheel;

    /// This should be called when any of the other parameters are changed
    void recalculateBotToWheel() {
        // See this paper for more info on how this matrix is derived:
        // http://people.idsia.ch/~foerster/2006/1/omnidrive_kiart_preprint.pdf

        // Factor WheelDist (R) into this matrix
        // clang-format off
        BotToWheel <<
            -sinf(WheelAngles[0]), cosf(WheelAngles[0]), WheelDist,
            -sinf(WheelAngles[1]), cosf(WheelAngles[1]), WheelDist,
            -sinf(WheelAngles[2]), cosf(WheelAngles[2]), WheelDist,
            -sinf(WheelAngles[3]), cosf(WheelAngles[3]), WheelDist;
        // Invert because our wheels spin opposite to paper
        BotToWheel *= -1;
        BotToWheel /= WheelRadius;
        // clang-format on
    }

    float DutyCycleMultiplier = 2.0f;
};

/// Model parameters for robot.  See RobotModel.cpp for values.
extern const RobotModel RobotModelControl;
