#pragma once

#define EIGEN_HAS_CXX11_MATH 0
#include <Eigen/Dense>
#include <array>
#include "Geometry2d/Util.hpp"
//#include "const-math.hpp"

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

    /// wheelSlip = ( I - BotToWheel*BotToWheel^+ ) * wheelSpeed
    Eigen::Matrix<double, 4, 4> SlipDetect;

    /// Slip vector in the motor matrix where the result of a movement
    /// yields no movement
    Eigen::Vector4d SlipVector;

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

    /// This should be called when any of the other parameters are changed
    /// Should be called after recalculateBotToWheel is called
    void recalculateSlipDetect() {
                     // I - DD^+, where D^+ = (D'*D)^-1 * D'
        SlipDetect = Eigen::MatrixXd::Identity(4, 4) - BotToWheel * (BotToWheel.transpose() * BotToWheel).inverse() * BotToWheel.transpose();

                     // Assume that that the Slip Detect Matrix is actually the has the null space basis as the rows with different scales
                     // This holds true in the matlab sim
        SlipVector = SlipDetect.row(0).transpose();
                     // This returns a 0 vector for some odd reason
                     //((Eigen::Matrix<double, 4, 4>)BotToWheel.fullPivLu().kernel()).col(0);
    }

    float DutyCycleMultiplier = 2.0f;
};

/// Model parameters for 2015 robot.  See RobotModel.cpp for values.
extern const RobotModel RobotModel2015;
