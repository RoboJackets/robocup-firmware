#include "modules/MotionControlModule.hpp"
#include "mtrain.hpp"
#include "rc-fshare/robot_model.hpp"
#include <math.h>
#include "MicroPackets.hpp"

extern DebugInfo debugInfo;

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
    if (!motionCommand->isValid /*|| !isRecentUpdate(motionCommand->lastUpdate)*/) {
        motorCommand->isValid = false;
        motorCommand->lastUpdate = HAL_GetTick();
    }

    // Fill data from shared mem
    Eigen::Matrix<double, 5, 1> measurements;
    measurements << 0, 0, 0, 0, 0;
    
    if (motorFeedback->isValid/** && isRecentUpdate(motorFeedback->lastUpdate)*/) {
        for (int i = 0; i < 4; i++) {
            measurements(i, 0) = motorFeedback->encoders[i];
        }
    }

    if (imuData->isValid/** && isRecentUpdate(imuData->lastUpdate)*/) {
        measurements(4, 0) = imuData->omegas[2]; // Z gyro
    }

    Eigen::Matrix<double, 3, 1> target;
    target << 0, 0, 0;
    
    if (motionCommand->isValid/* && isRecentUpdate(motionCommand->lastUpdate)*/) {
        target << motionCommand->bodyXVel,
                  motionCommand->bodyYVel,
                  motionCommand->bodyWVel;
    }


    // Run estimators
    robotEstimator.predict(prevCommand);

    // Only use the feedback if we have good inputs
    // todo figure out where the nan's are coming in
    if (motorFeedback->isValid && imuData->isValid &&
        !isnan(measurements(0,0)) &&
        !isnan(measurements(1,0)) &&
        !isnan(measurements(2,0)) &&
        !isnan(measurements(3,0)) &&
        !isnan(measurements(4,0))){

        robotEstimator.update(measurements);
        //printf("%u,w1,%7.4f,w2,%7.4f,w3,%7.4f,w4,%7.4f,gz,%7.4f\r\n",
        //    HAL_GetTick(), measurements(0,0), measurements(1,0), measurements(2,0), measurements(3,0), measurements(4,0));
    }
    
    Eigen::Matrix<double, 3, 1> currentState;
    robotEstimator.getState(currentState);


    //printf("%u,x,%7.4f,y,%7.4f,w,%7.4f\r\n",
    //        HAL_GetTick(), currentState(0,0), currentState(1,0), currentState(2,0));
    
    

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
            motorCommand->wheels[i] = motorCommands(i, 0);
        }
        motorCommand->dribbler = dribblerCommand;

    } else {

        // rip battery
        // stop
        for (int i = 0; i < 4; i++) {
            motorCommand->wheels[i] = 0.0f;
        }
        motorCommand->dribbler = 0;
    }
}

bool MotionControlModule::isRecentUpdate(uint32_t lastUpdateTime) {
    return (HAL_GetTick() - lastUpdateTime) < COMMAND_TIMEOUT;
}