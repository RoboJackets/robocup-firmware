#include "MotionControl.hpp"
#include <cmath>

namespace motion {

using namespace std::chrono;

MotorVoltage MotionControl::update_control(Odometry odometry, time_point now) {
    // Run the estimator
    estimator.predict(last_u);
    estimator.update_odometry(odometry);

    // Extrapolate goals from the waypoint
    double time_since = duration_cast<microseconds>(now - waypoint.time).count() * 1e-6;
    RobotPose rx = waypoint.pose
        + waypoint.velocity * time_since
        + 0.5 * waypoint.acceleration * time_since * time_since;
    RobotPose rv = waypoint.velocity + waypoint.acceleration * time_since;
    RobotPose ra = waypoint.acceleration;

    // Get estimate from the filter
    RobotPose x = extract_pose(estimator.get_x());
    RobotTwist v = extract_velocity(estimator.get_x());

    last_u = controller.calculate(x, v, rx, rv, ra);

    return last_u;
}

void MotionControl::update_camera(Camera camera, time_point camera_time, time_point now) {
    double time_seconds = duration_cast<microseconds>(now - camera_time).count() * 1e-6;
    estimator.update_camera(camera, time_seconds);
}

void MotionControl::set_waypoint(Waypoint new_waypoint) {
    waypoint = new_waypoint;
}

};
