#include "RobotEstimator.hpp"

namespace motion {

RobotEstimator::RobotEstimator(RobotPose x0, RobotTwist v0) {
    K_odom = (Matrix<3, 5>() <<
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            ).finished();
    K_cam = (Matrix<6, 3>() <<
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0,
            0, 0, 0
            ).finished();
    x = pack_pose_velocity(x0, v0);
}

void RobotEstimator::predict(MotorVoltage u) {
    RobotPose pose = extract_pose(x);
    RobotTwist velocity = extract_velocity(x);
    RobotTwist acceleration = robot_model.forward_dynamics_world(pose, velocity, u);
    x += pack_pose_velocity(
            velocity * kLoopTime + acceleration * kLoopTime * kLoopTime,
            acceleration * kLoopTime);
}

void RobotEstimator::update_odometry(Odometry odometry) {
    // Rotate into the body frame to do the Kalman update,
    // then rotate back to apply changes.
    Matrix<3> rotation_matrix = make_rotation_matrix(x(2));

    RobotTwist body_velocity = rotation_matrix.transpose() * extract_velocity(x);
    Odometry expected = robot_model.read_odom(body_velocity);

    x += pack_pose_velocity(
            RobotPose::Zero(), rotation_matrix * K_odom * (odometry - expected));
}

void RobotEstimator::update_camera(Camera camera_measurements, double time_since) {
    // TODO(Kyle): Figure out something clever to do with camera times here.
    RobotPose pose = extract_pose(x);
    RobotTwist velocity = extract_velocity(x);
    Camera expected = robot_model.read_cam(pose, velocity);
    x += K_cam * (camera_measurements - expected);
}

}
