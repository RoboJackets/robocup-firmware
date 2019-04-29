#pragma once

#include <chrono>

#include "RobotModel.hpp"
#include "RobotEstimator.hpp"
#include "RobotController.hpp"

namespace motion {

using clock = std::chrono::steady_clock;
using time_point = std::chrono::time_point<clock>;

struct Waypoint {
    RobotPose pose;
    RobotTwist velocity;
    RobotTwist acceleration;
    time_point time;
};

class MotionControl {
public:
    // Called at 1000hz to update the filter based on encoder values
    // and calculate torque goals.
    MotorVoltage update_control(Odometry odometry, time_point now);

    // Called at ~60hz to update with new camera input.
    void update_camera(Camera camera, time_point camera_time, time_point now);

    void set_waypoint(Waypoint waypoint);

private:
    RobotEstimator estimator{RobotPose::Zero(), RobotTwist::Zero()};
    RobotController controller;

    MotorVoltage last_u = MotorVoltage::Zero();

    Waypoint waypoint;
};

};
