#include "modules/MotionControlModule.hpp"
#include "mtrain.hpp"

MotionControlModule::MotionControlModule(BatteryVoltage *const batteryVoltage,
                                         IMUData *const imuData,
                                         MotionCommand *const motionCommand,
                                         MotorFeedback *const motorFeedback,
                                         MotorCommand *const motorCommand)
    : batteryVoltage(batteryVoltage), imuData(imuData),
      motionCommand(motionCommand), motorFeedback(motorFeedback),
      motorCommand(motorCommand),
      dribblerController(period), // todo check if namespace is needed
      robotController(period),
      robotEstimator(period) {
    
    // todo init estimator / regulator
    prevCommand << 0, 0, 0, 0;

    motorCommand->isValid = false;
    motorCommand->lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommand->wheels[i] = 0;
    }
    motorCommand->dribbler = 0;
}

void MotionControlModule::entry(void) {
    // No radio comm in a little while
    // return and die
    if (!motionCommand->isValid || !isRecentUpdate(motionCommand->lastUpdate)) {
        motorCommand->isValid = false;
        motorCommand->lastUpdate = HAL_GetTick();
    }

    // Fill data from shared mem
    Eigen::Matrix<double, 5, 1> measurements;
    measurements << 0, 0, 0, 0, 0;
    
    if (motorFeedback->isValid && isRecentUpdate(motorFeedback->lastUpdate)) {
        for (int i = 0; i < 4; i++) {
            measurements(i, 0) = motorFeedback->encoders[i];
        }
    }

    if (imuData->isValid && isRecentUpdate(imuData->lastUpdate)) {
        measurements(4, 0) = imuData->omegas[2]; // Z gyro
    }

    Eigen::Matrix<double, 3, 1> target;
    target << 0, 0, 0;
    
    if (motionCommand->isValid && isRecentUpdate(motionCommand->lastUpdate)) {
        target << motionCommand->bodyXVel,
                  motionCommand->bodyYVel,
                  motionCommand->bodyWVel;
    }



    // Run estimators
    robotEstimator.predict(prevCommand);
    robotEstimator.update(measurements);

    Eigen::Matrix<double, 3, 1> currentState;
    robotEstimator.getState(currentState);



    // Run controllers
    uint8_t dribblerCommand = 0;
    dribblerController.calculate(motionCommand->dribbler, dribblerCommand);

    Eigen::Matrix<double, 4, 1> motorCommands;
    robotController.calculate(currentState, target, motorCommands);
    prevCommand = motorCommands;



    motorCommand->isValid = true;
    motorCommand->lastUpdate = HAL_GetTick();

    // Good to run motors
    // todo Check stall and motor errors?
    if (batteryVoltage->isValid && !batteryVoltage->isCritical) {

        // set motors to real targets
        for (int i = 0; i < 4; i++) {
            motorCommand->wheels[i] = 0;
        }
        motorCommand->dribbler = dribblerCommand;

    } else {

        // rip battery
        // stop
        for (int i = 0; i < 4; i++) {
            motorCommand->wheels[i] = motorCommands(i, 0);
        }
        motorCommand->dribbler = 0;

    }
}

bool MotionControlModule::isRecentUpdate(uint32_t lastUpdateTime) {
    return (HAL_GetTick() - lastUpdateTime) < COMMAND_TIMEOUT;
}