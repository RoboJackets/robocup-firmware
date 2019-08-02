#include "modules/RadioModule.hpp"
#include "iodefs.h"

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
      radioError(radioError), link(),
      secondRadioCS(RADIO_R1_CS) {

    secondRadioCS = 1;

    // todo fill out more kicker stuff
    kickerCommand->isValid = false;
    kickerCommand->lastUpdate = 0;
    kickerCommand->shootMode = KickerCommand::ShootMode::KICK;
    kickerCommand->triggerMode = KickerCommand::TriggerMode::OFF;
    kickerCommand->kickStrength = 0;

    motionCommand->isValid = false;
    motionCommand->lastUpdate = 0;
    motionCommand->bodyXVel = 0.0f;
    motionCommand->bodyYVel = 0.0f;
    motionCommand->bodyWVel = 0.0f;
    motionCommand->dribbler = 0;

    radioError->isValid = false;
    radioError->lastUpdate = 0;
    radioError->hasError = false;

    printf("INFO: Radio initialized\r\n");
}

void RadioModule::entry(void) {
    // Just check to see if our robot id is valid
    // That way we don't conflict with other robots on the network
    // that are working
    if (batteryVoltage->isValid && fpgaStatus->isValid && robotID->isValid)
      link.send(*batteryVoltage, *fpgaStatus, *kickerInfo, *robotID);

    // Try read
    // Clear buffer of old packets such that we can get the lastest packet
    // If you don't do this there is a significant lag of 300ms or more
    while (link.receive(*kickerCommand, *motionCommand)) {
      kickerCommand->isValid = true;
      kickerCommand->lastUpdate = HAL_GetTick();

      motionCommand->isValid = true;
      motionCommand->lastUpdate = HAL_GetTick();
    }

    radioError->isValid = true;
    radioError->lastUpdate = HAL_GetTick();
    radioError->hasError = false;
}