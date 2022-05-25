#include "modules/MotionControlModule.hpp"
#include "mtrain.hpp"
#include "rc-fshare/robot_model.hpp"
#include <math.h>
#include "MicroPackets.hpp"
#include "DigitalOut.hpp"
#include <algorithm>

MotionControlModule::MotionControlModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                                         LockedStruct<IMUData>& imuData,
                                         LockedStruct<MotionCommand>& motionCommand,
                                         LockedStruct<MotorFeedback>& motorFeedback,
                                         LockedStruct<MotorCommand>& motorCommand,
                                         LockedStruct<DebugInfo>& debugInfo)
    : GenericModule(kPeriod, "motion", kPriority, 1024),
      batteryVoltage(batteryVoltage), imuData(imuData),
      motionCommand(motionCommand), motorFeedback(motorFeedback),
      motorCommand(motorCommand),
      debugInfo(debugInfo),
      dribblerController(kPeriod.count()),
      robotController(kPeriod.count() * 1000),
      robotEstimator(kPeriod.count() * 1000) {

    prevCommand << 0, 0, 0, 0;

    auto motorCommandLock = motorCommand.unsafe_value();
    motorCommandLock->isValid = false;
    motorCommandLock->lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommandLock->wheels[i] = 0;
    }
    motorCommandLock->dribbler = 0;
}

void MotionControlModule::entry() {
    auto motion_command = motionCommand.lock().value();
    auto motor_feedback = motorFeedback.lock().value();
    auto imu_data = imuData.lock().value();
    auto battery_voltage_data = batteryVoltage.lock().value();

    MotorCommand motor_command;

    DebugFrame frame{};
    frame.frame_time_ms = xTaskGetTickCount();
    frame.gyro_z_radps = imu_data.omegas[2];
    frame.measured_acceleration[0] = imu_data.accelerations[0];
    frame.measured_acceleration[1] = imu_data.accelerations[1];
    frame.measured_acceleration[2] = imu_data.accelerations[2];

    // No radio comm in a little while. Return and die.
    if (!motion_command.isValid || !isRecentUpdate(motion_command.lastUpdate)) {
        motor_command.isValid = false;
        motor_command.lastUpdate = HAL_GetTick();
    }

    // Fill data from shared mem
    Eigen::Matrix<float, 5, 1> measurements;
    Eigen::Matrix<float, 4, 1> currentWheels;
    measurements << 0, 0, 0, 0, 0;
    currentWheels << 0, 0, 0, 0;

    if (motor_feedback.isValid && isRecentUpdate(motor_feedback.lastUpdate)) {
        for (int i = 0; i < 4; i++) {
            if (!isnan(measurements(i,0))) {
                measurements(i, 0) = motor_feedback.encoders[i];
                currentWheels(i, 0) = motor_feedback.encoders[i];
            } else {
                measurements(i, 0) = 0;
                currentWheels(i, 0) = 0;
            }
        }
    }

    if (imu_data.isValid && isRecentUpdate(imu_data.lastUpdate)) {
        measurements(4, 0) = imu_data.omegas[2]; // Z gyro
    }

    // Update targets
    Eigen::Matrix<float, 3, 1> targetState;
    targetState << 0, 0, 0;

    if (motion_command.isValid && isRecentUpdate(motion_command.lastUpdate)) {
        targetState << motion_command.bodyXVel,
                       motion_command.bodyYVel,
                       motion_command.bodyWVel;
    }

    // Run estimators
    robotEstimator.predict(prevCommand);

    // Only use the feedback if we have good inputs
    // NAN's most likely came from the divide by dt in the fpga
    // which was 0, resulting in bad behavior
    // Leaving this hear until someone can test, then remove
    // this
    // - Joe Aug 2019
    if (motor_feedback.isValid && // imuData->isValid &&
        !isnan(measurements(0,0)) &&
        !isnan(measurements(1,0)) &&
        !isnan(measurements(2,0)) &&
        !isnan(measurements(3,0)) &&
        !isnan(measurements(4,0))) {
        robotEstimator.update(measurements);
    } else {
        // Assume we're stopped.
        robotEstimator.update(Eigen::Matrix<float, 5, 1>::Zero());
    }

    Eigen::Matrix<float, 3, 1> currentState;
    robotEstimator.getState(currentState);

    for (int i = 0; i < 3; i++) {
        frame.filtered_velocity[i] = currentState(i);
    }

    // Run controllers
    uint8_t dribblerCommand = 0;
    dribblerController.calculate(motion_command.dribbler, dribblerCommand);

    Eigen::Matrix<float, 4, 1> targetWheels;
    Eigen::Matrix<float, 4, 1> motorCommands;

    robotController.calculateBody(currentState, targetState, targetWheels);
    robotController.calculateWheel(currentWheels, targetWheels, motorCommands);

    prevCommand = motorCommands;

    motor_command.isValid = true;
    motor_command.lastUpdate = HAL_GetTick();

    // Good to run motors
    // todo Check stall and motor errors?
    if (battery_voltage_data.isValid && !battery_voltage_data.isCritical) {

        // set motors to real targets
        for (int i = 0; i < 4; i++) {
            motor_command.wheels[i] = motorCommands(i, 0);
        }
        motor_command.dribbler = dribblerCommand;
    } else {

        // rip battery
        // stop
        for (int i = 0; i < 4; i++) {
            motor_command.wheels[i] = 0.0f;
        }
        motor_command.dribbler = 0;
    }

    for (int i = 0; i < 4; i++) {
        frame.motor_outputs_pwm[i] = motor_command.wheels[i];
        frame.wheel_speeds_radps[i] = currentWheels(i);
    }

    {
        auto debugInfoLock = debugInfo.lock();
        if (debugInfoLock->debug_frames.size() < 16) {
            debugInfoLock->debug_frames.push_back(frame);
        }
    }

    {
        auto motorCommandLock = motorCommand.lock();
        motorCommandLock.value() = motor_command;
    }
}

bool MotionControlModule::isRecentUpdate(uint32_t lastUpdateTime) {
    return (HAL_GetTick() - lastUpdateTime) < COMMAND_TIMEOUT;
}
