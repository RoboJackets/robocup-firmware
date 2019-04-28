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
    x.block<3, 1>(0, 0) = x0;
    x.block<3, 1>(3, 0) = v0;
}

void RobotEstimator::predict(Vector<4> u) {
    RobotTwist acceleration = robot_model.forward_dynamics_world(
            x.block<3, 1>(0, 0), x.block<3, 1>(3, 0), u);
    x.block<3, 1>(0, 0) += x.block<3, 1>(3, 0) * kLoopTime
        + acceleration * kLoopTime * kLoopTime;
    x.block<3, 1>(3, 0) += acceleration * kLoopTime;
}

void RobotEstimator::update_odometry(Odometry odometry) {
    // Rotate into the body frame to do the Kalman update,
    // then rotate back to apply changes.
    Matrix<3> rotation_matrix = make_rotation_matrix(x(2));

    RobotTwist body_velocity = rotation_matrix.transpose() * x.block<3, 1>(3, 0);
    Odometry expected = robot_model.read_odom(body_velocity);

    x.block<3, 1>(3, 0) += rotation_matrix * K_odom * (odometry - expected);
}

void RobotEstimator::update_camera(Camera camera_measurements, double time_since) {
    // TODO(Kyle): Figure out something clever to do with camera times here.
    Camera expected = robot_model.read_cam(x.block<3, 1>(0, 0), x.block<3, 1>(3, 0));
    x += K_cam * (camera_measurements - expected);
}

}
