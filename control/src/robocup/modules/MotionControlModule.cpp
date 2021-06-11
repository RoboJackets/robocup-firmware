#include "modules/MotionControlModule.hpp"
#include "mtrain.hpp"
#include "rc-fshare/robot_model.hpp"
#include <math.h>
#include "MicroPackets.hpp"
#include "DigitalOut.hpp"
#include <algorithm>

extern DebugInfo debugInfo;

MotionControlModule::MotionControlModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                                         LockedStruct<IMUData>& imuData,
                                         LockedStruct<MotionCommand>& motionCommand,
                                         LockedStruct<MotorFeedback>& motorFeedback,
                                         LockedStruct<MotorCommand>& motorCommand)
    : GenericModule(kPeriod, "motion", kPriority, 1024),
      batteryVoltage(batteryVoltage), imuData(imuData),
      motionCommand(motionCommand), motorFeedback(motorFeedback),
      motorCommand(motorCommand),
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
    auto motionCommandLock = motionCommand.lock();
    auto motorCommandLock = motorCommand.lock();
    auto motorFeedbackLock = motorFeedback.lock();
    auto imuDataLock = imuData.lock();
    auto batteryVoltageLock = batteryVoltage.lock();

    // No radio comm in a little while. Return and die.
    if (!motionCommandLock->isValid || !isRecentUpdate(motionCommandLock->lastUpdate)) {
        motorCommandLock->isValid = false;
        motorCommandLock->lastUpdate = HAL_GetTick();
    }

    // Fill data from shared mem
    Eigen::Matrix<float, 5, 1> measurements;
    Eigen::Matrix<float, 4, 1> currentWheels;
    measurements << 0, 0, 0, 0, 0;
    currentWheels << 0, 0, 0, 0;

    if (motorFeedbackLock->isValid && isRecentUpdate(motorFeedbackLock->lastUpdate)) {
        for (int i = 0; i < 4; i++) {
            if (!isnan(measurements(i,0))) {
                measurements(i, 0) = motorFeedbackLock->encoders[i];
                currentWheels(i, 0) = motorFeedbackLock->encoders[i];
            } else {
                measurements(i, 0) = 0;
                currentWheels(i, 0) = 0;
            }
        }
    }

    if (imuDataLock->isValid && isRecentUpdate(imuDataLock->lastUpdate)) {
        measurements(4, 0) = imuDataLock->omegas[2]; // Z gyro
    }

    // Update targets
    Eigen::Matrix<float, 3, 1> targetState;
    targetState << 0, 0, 0;

    if (motionCommandLock->isValid && isRecentUpdate(motionCommandLock->lastUpdate)) {
        targetState << motionCommandLock->bodyXVel,
                       motionCommandLock->bodyYVel,
                       motionCommandLock->bodyWVel;
    }

    // Run estimators
    robotEstimator.predict(prevCommand);


    // Only use the feedback if we have good inputs
    // NAN's most likely came from the divide by dt in the fpga
    // which was 0, resulting in bad behavior
    // Leaving this hear until someone can test, then remove
    // this
    // - Joe Aug 2019
    if (motorFeedbackLock->isValid && // imuData->isValid &&
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

    // Run controllers
    uint8_t dribblerCommand = 0;
    dribblerController.calculate(motionCommandLock->dribbler, dribblerCommand);

    Eigen::Matrix<float, 4, 1> targetWheels;
    Eigen::Matrix<float, 4, 1> motorCommands;

    robotController.calculateBody(currentState, targetState, targetWheels);
    robotController.calculateWheel(currentWheels, targetWheels, motorCommands);

    prevCommand = motorCommands;

    motorCommandLock->isValid = true;
    motorCommandLock->lastUpdate = HAL_GetTick();

    // Good to run motors
    // todo Check stall and motor errors?
    if (batteryVoltageLock->isValid && !batteryVoltageLock->isCritical) {

        // set motors to real targets
        for (int i = 0; i < 4; i++) {
            motorCommandLock->wheels[i] = motorCommands(i, 0);
        }
        motorCommandLock->dribbler = dribblerCommand;
    } else {

        // rip battery
        // stop
        for (int i = 0; i < 4; i++) {
            motorCommandLock->wheels[i] = 0.0f;
        }
        motorCommandLock->dribbler = 0;
    }
}

bool MotionControlModule::isRecentUpdate(uint32_t lastUpdateTime) {
    return (HAL_GetTick() - lastUpdateTime) < COMMAND_TIMEOUT;
}
