#include "modules/MotionControlModule.hpp"
#include "mtrain.hpp"

MotionControlModule::MotionControlModule(BatteryVoltage *const batteryVoltage,
                                         IMUData *const imuData,
                                         MotionCommand *const motionCommand,
                                         MotorFeedback *const motorFeedback,
                                         MotorCommand *const motorCommand)
    : batteryVoltage(batteryVoltage), imuData(imuData),
      motionCommand(motionCommand), motorFeedback(motorFeedback),
      motorCommand(motorCommand) /**estimator regulator**/ {
    
    // todo init estimator / regulator

    motorCommand->isValid = false;
    motorCommand->lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorCommand->wheels[i] = 0;
    }
    motorCommand->dribbler = 0;
}

void MotionControlModule::entry(void) {
    // grab data and make sure good
    // pass into estimator
    // run regulator on state and target
    // pull out data

    motorCommand->isValid = true;
    motorCommand->lastUpdate = HAL_GetTick();

    // rip battery
    // stop
    for (int i = 0; i < 4; i++) {
        motorCommand->wheels[i] = motionCommand->dribbler;
    }
    motorCommand->dribbler = motionCommand->dribbler;
    //motorCommand->dribbler = motionCommand->dribbler;
    // Good to run motors
    if (batteryVoltage->isValid && !batteryVoltage->isCritical) {
        // set motors to real targets
    } else {
        // rip battery
        // stop
        for (int i = 0; i < 4; i++) {
            motorCommand->wheels[i] = motionCommand->dribbler;
        }
        motorCommand->dribbler = 0;
    }
    
}