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
    if (!motionCommand->isValid ||
        (HAL_GetTick() - motionCommand->lastUpdate) > COMMAND_TIMEOUT) {
        
        motorCommand->isValid = false;
        motorCommand->lastUpdate = HAL_GetTick();
    }

    // run estimators

    // Run controllers
    uint8_t dribblerCommand = 0;
    dribblerController.calculate(motionCommand->dribbler, dribblerCommand);

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
            motorCommand->wheels[i] = motionCommand->dribbler;
        }
        motorCommand->dribbler = 0;

    }
    
}