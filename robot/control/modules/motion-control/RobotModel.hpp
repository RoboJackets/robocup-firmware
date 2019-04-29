#pragma once

#define EIGEN_HAS_CXX11_MATH 0
#include <Eigen/Dense>

namespace motion {

template<int N, int M = N>
using Matrix = Eigen::Matrix<double, N, M>;

template<int N>
using Vector = Matrix<N, 1>;

constexpr double to_radians(double degrees) {
    return degrees * M_PI / 180;
}

constexpr double kLoopTime = 0.005;

// Configuration (all values in world space):
// q = [ x y theta].T
using RobotPose = Vector<3>;

// q' = [ vx vy theta].T
using RobotTwist = Vector<3>;

// Input:
// u = [ tau_1 tau_2 tau_3 tau_4 ].T
using MotorVoltage = Vector<4>;

// Odometry Output:
// z_o = [ omega_1 omega_2 omega_3 omega_4 gyro_rate ]
using Odometry = Vector<5>;

// Camera Output:
// z_c = [ x y phi ]
using Camera = Vector<3>;

Vector<6> pack_pose_velocity(RobotPose x, RobotTwist v);
RobotPose extract_pose(Vector<6> state);
RobotTwist extract_velocity(Vector<6> state);

class RobotModel {
public:
    RobotModel();

    void update_constants(double phi);

    // Inputs and outputs are in the fixed frame coinciding
    // with the body frame at this instant in time.
    RobotTwist forward_dynamics(RobotTwist v, MotorVoltage u);
    MotorVoltage inverse_dynamics(RobotTwist v, RobotTwist a);

    // Inputs and outputs in the world frame.
    RobotTwist forward_dynamics_world(RobotPose x, RobotTwist v, MotorVoltage u);
    MotorVoltage inverse_dynamics_world(RobotPose x, RobotTwist v, RobotTwist a);

    // Get the odometry readings for a certain body velocity
    Odometry read_odom(RobotTwist body_velocity);

    // Get the camera readings given by a certain state
    Camera read_cam(RobotPose x, RobotTwist v);

private:
    Matrix<4, 3> G, GT_inv;

    Matrix<3> wheel_friction_mat;
    Matrix<3> M;

    Matrix<3, 3> Z;
    Matrix<3, 3> Z_inv;

    Matrix<5, 6> C_odom;
    Matrix<3, 6> C_cam;

    Matrix<6> Q;
    Matrix<5> R_odom;
    Matrix<3> R_cam;
};

constexpr double kMotorTorqueConstant = 0.0335;
constexpr double kMotorSpeedConstant = 0.0335;
constexpr double kMotorResistance = 1.03;
constexpr double kMotorRotorInertia = 135e-7;

constexpr double kRobotMass = 6.35;
constexpr double kRobotRadius = 0.0789;
constexpr double kGearRatio = 20.0 / 60.0;
constexpr double kRobotInertia = 0.5 * kRobotMass * kRobotRadius * kRobotRadius;
constexpr double kWheelRadius = 0.02711;
constexpr double kWheelInertia = 0.00002516704;
constexpr double kRotorInertia = 135e-7;
constexpr double kWheelTheta[4] = {
    // Front left
    to_radians(60),
    // Rear left
    to_radians(129),
    // Rear right
    to_radians(-129),
    // Front right
    to_radians(-60)
};

Matrix<3> make_rotation_matrix(double phi);

}
