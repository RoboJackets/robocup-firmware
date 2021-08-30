#include "motion-control/RobotController.hpp"
#include "rc-fshare/robot_model.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp"
#include <cstdlib>

extern DebugInfo debugInfo;

RobotController::RobotController(uint32_t dt_us) {
    // Proportional constants.
    BodyKp << 0.8, 0.8, 1.5;

    //WheelKp = 0.01;
    max_acceleration_ << 4.0, 3.0, 30.0;
}

void RobotController::calculateBody(Eigen::Matrix<float, numStates, 1> pv,
                                    Eigen::Matrix<float, numStates, 1> sp,
                                    Eigen::Matrix<float, numWheels, 1>& outputs) {
    Eigen::Matrix<float, numWheels, numStates> G = RobotModel::get().BotToWheel.cast<float>();
    Eigen::Matrix<float, numStates, 1> accel = (sp - last_body_) / dt;

    accel /= std::max<float>(1.0, accel.cwiseAbs().cwiseQuotient(max_acceleration_).maxCoeff());
    sp = last_body_ + accel * dt;
    last_body_ = sp;

    Eigen::Matrix<float, numStates, 1> error = sp - pv;
    outputs = G * (BodyKp.cwiseProduct(error) + sp);
}

void RobotController::calculateWheel(Eigen::Matrix<float, numWheels, 1> pv,
                                     Eigen::Matrix<float, numWheels, 1> sp,
                                     Eigen::Matrix<float, numWheels, 1>& outputs) {
    for (int i = 0; i < numWheels; i++) {
        if (!std::isfinite(last_wheels_(i)) || std::isnan(last_wheels_(i))) {
            printf("Warning: last wheels had %f\n", last_wheels_(i));
            last_wheels_ = Eigen::Matrix<float, numWheels, 1>::Zero();
        }
    }

    Eigen::Matrix<float, numWheels, 1> accel = (sp - last_wheels_) / dt;

    accel /= std::max<float>(1.0, accel.cwiseAbs().maxCoeff() / kMaxWheelAcceleration);
    sp = last_wheels_ + accel * dt;
    sp /= std::max<float>(1.0, sp.cwiseAbs().maxCoeff() / kMaxWheelSpeed);

    Eigen::Matrix<float, numWheels, 1> error = sp - pv;
    outputs = sp * RobotModel::get().kSpeedToDutyCycle + 0.01 * error;

    last_wheels_ = sp;
}
