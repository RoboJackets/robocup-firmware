#include "modules/RadioModule.hpp"

RadioModule::RadioModule(BatteryVoltage *const batteryVoltage,
                         FPGAStatus *const fpgaStatus,
                         KickerInfo *const kickerInfo,
                         RobotID *const robotID,
                         KickerCommand *const kickerCommand,
                         MotionCommand *const motionCommand,
                         RadioError *const radioError)
    : batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), robotID(robotID),
      kickerCommand(kickerCommand), motionCommand(motionCommand),
      radioError(radioError) /**, radio **/ {

    // todo init radio

    // todo fill out more kicker stuff
    kickerCommand->isValid = false;
    kickerCommand->lastUpdate = 0;

    motionCommand->isValid = false;
    motionCommand->lastUpdate = 0;
    motionCommand->bodyXVel = 0.0f;
    motionCommand->bodyYVel = 0.0f;
    motionCommand->bodyWVel = 0.0f;
    motionCommand->dribbler = 0;

    radioError->isValid = false;
    radioError->lastUpdate = 0;
    radioError->hasError = false;
}

void RadioModule::entry(void) {
    // if all data is valid
    // send it over
    
    // Try read
    // set data correctly
}