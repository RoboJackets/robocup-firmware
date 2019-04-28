#pragma once

#include "RobotModel.hpp"

namespace motion {

class RobotEstimator {
public:
    // The estimator is linearized around phi=0.
    // Do camera corrections in world space.
    // Do robot-space corrections in body space and rotate back into world space.
    explicit RobotEstimator(RobotPose x0, RobotTwist v0);

    void predict(MotorVoltage u);
    void update_odometry(Odometry odometry);
    void update_camera(Camera camera_measurements, double time_since);

    Vector<6> get_x() const { return x; }

private:
    RobotModel robot_model;

    // We use a steady-state Kalman filter to save on compute,
    // because otherwise we'd be doing a lot of large matrix
    // multiplications (covariance is 6x6). Also, almost everything
    // we're doing is in the steady-state anyway (except for the
    // asymmetry created by nonlinearity, which _should_ hopefully
    // be negligeable). TODO(Kyle): Make sure these assumptions are
    // warranted.
    Matrix<3, 5> K_odom;
    Matrix<6, 3> K_cam;

    Vector<6> x;
};

}
