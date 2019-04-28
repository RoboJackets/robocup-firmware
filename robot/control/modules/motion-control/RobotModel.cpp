#include "RobotModel.hpp"
#include <cmath>

namespace motion {

Matrix<3> make_rotation_matrix(double phi) {
    return (Matrix<3>() <<
            std::cos(phi), -std::sin(phi), 0,
            std::sin(phi), std::cos(phi), 0,
            0, 0, 1
        ).finished();
}

constexpr double radians(double degrees) {
    return degrees * M_PI / 180;
}

RobotModel::RobotModel() {
    // Geometry matrix
    G = 1 / kWheelRadius * (Matrix<4, 3>() <<
            -std::sin(kWheelTheta[0]), std::cos(kWheelTheta[0]), kRobotRadius,
            -std::sin(kWheelTheta[1]), std::cos(kWheelTheta[1]), kRobotRadius,
            -std::sin(kWheelTheta[2]), std::cos(kWheelTheta[2]), kRobotRadius,
            -std::sin(kWheelTheta[3]), std::cos(kWheelTheta[3]), kRobotRadius
        ).finished();

    M = Vector<3>(kRobotMass, kRobotMass, kRobotInertia).asDiagonal();

    // Calculate the pseudo-inverse of G
    Eigen::CompleteOrthogonalDecomposition<Eigen::MatrixXd> GT_cod(G.transpose());
    GT_inv = GT_cod.pseudoInverse();

    // Calculate the effective wheel inertia, at the output shaft.
    double wheel_inertia = kWheelInertia + kRotorInertia / kGearRatio / kGearRatio;

    // Calculate the total effective mass matrix of the robot, including
    // body mass/inertia and additional wheel/motor inertia.
    Z = M + G.transpose() * wheel_inertia * G;
    Z_inv = Z.inverse();

    wheel_friction_mat = G.transpose() * Matrix<4, 4>::Identity() * G;
}

RobotTwist RobotModel::forward_dynamics(RobotTwist v, MotorVoltage u) {
    // Calculate motor torques
    Vector<4> motor_velocity = G * v / kGearRatio;
    Vector<4> motor_torque =
        kMotorTorqueConstant * (u - kMotorSpeedConstant * motor_velocity) / kMotorResistance;

    Vector<4> wheel_torque = motor_torque / kGearRatio;
    RobotEffort effort = G.transpose() * wheel_torque;

    Matrix<3, 3> coriolis = (Matrix<3, 3>() <<
        0, -v(2), 0,
        v(2), 0, 0,
        0, 0, 0
        ).finished();

    Matrix<3, 3> V = wheel_friction_mat + coriolis * M;
    return Z_inv * (effort - V * v);
}

MotorVoltage RobotModel::inverse_dynamics(RobotTwist v, RobotTwist a) {
    Vector<4> wheel_velocity = G * v;

    Matrix<3, 3> coriolis = (Matrix<3, 3>() <<
        0, -v(2), 0,
        v(2), 0, 0,
        0, 0, 0
        ).finished();

    Matrix<3, 3> V = wheel_friction_mat + coriolis * M;

    // Wheel torques
    Vector<4> net_torque = GT_inv * Z * a;

    // Torque from ficticious forces and friction.
    Vector<4> internal_torque = GT_inv * V * GT_inv.transpose() * wheel_velocity;

    // Motor torque
    Vector<4> applied_torque = (net_torque + internal_torque) * kGearRatio;

    Vector<4> motor_velocity = wheel_velocity / kGearRatio;

    // Motor voltage
    // T = Kt/R(u - Kv*v)
    // u = RT/Kt+Kv*v
    return applied_torque * kMotorResistance / kMotorTorqueConstant
        + kMotorSpeedConstant * motor_velocity;
}

RobotTwist RobotModel::forward_dynamics_world(RobotPose x, RobotTwist v, MotorVoltage u) {
    Matrix<3, 3> coriolis = (Matrix<3, 3>() <<
        0, -v(2), 0,
        v(2), 0, 0,
        0, 0, 0
        ).finished();
    Matrix<3> rotation_matrix = make_rotation_matrix(x(2));
    RobotTwist velocity_body = rotation_matrix.transpose() * v;
    RobotTwist acceleration_body = forward_dynamics(velocity_body, u);
    return coriolis * v + rotation_matrix * acceleration_body;
}

MotorVoltage RobotModel::inverse_dynamics_world(RobotPose x, RobotTwist v, RobotTwist a) {
    Matrix<3, 3> coriolis = (Matrix<3, 3>() <<
        0, -v(2), 0,
        v(2), 0, 0,
        0, 0, 0
        ).finished();

    Matrix<3, 3> rotation_matrix = make_rotation_matrix(x(2));
    RobotTwist velocity_body = rotation_matrix.transpose() * v;
    RobotTwist acceleration_body = rotation_matrix.transpose() * a
        - coriolis * velocity_body;
    return inverse_dynamics(velocity_body, acceleration_body);
}

Vector<5> RobotModel::read_odom(RobotTwist body_velocity) {
    Vector<4> wheel_velocity = G * body_velocity;
    return (Vector<5>() <<
            wheel_velocity(0),
            wheel_velocity(1),
            wheel_velocity(2),
            wheel_velocity(3),
            body_velocity(2)).finished();
}

Vector<3> RobotModel::read_cam(RobotPose x, RobotTwist v) {
    // TODO(Kyle): Subtract `camera lag * v` from x to extrapolate
    // backwards to compensate.
    return x;
}

}
