#include "modules/RadioModule.hpp"
#include "iodefs.h"

RadioModule::RadioModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                         LockedStruct<FPGAStatus>& fpgaStatus,
                         LockedStruct<KickerInfo>& kickerInfo,
                         LockedStruct<RobotID>& robotID,
                         LockedStruct<KickerCommand>& kickerCommand,
                         LockedStruct<MotionCommand>& motionCommand,
                         LockedStruct<RadioError>& radioError)
    : GenericModule(kPeriod, "radio", kPriority),
      batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), robotID(robotID),
      kickerCommand(kickerCommand), motionCommand(motionCommand),
      radioError(radioError), link(),
      secondRadioCS(RADIO_R1_CS) {

    secondRadioCS = 1;

    // todo fill out more kicker stuff
    auto kickerCommandLock = kickerCommand.unsafe_value();
    kickerCommandLock->isValid = false;
    kickerCommandLock->lastUpdate = 0;
    kickerCommandLock->shootMode = KickerCommand::ShootMode::KICK;
    kickerCommandLock->triggerMode = KickerCommand::TriggerMode::OFF;
    kickerCommandLock->kickStrength = 0;

    auto motionCommandLock = motionCommand.unsafe_value();
    motionCommandLock->isValid = false;
    motionCommandLock->lastUpdate = 0;
    motionCommandLock->bodyXVel = 0.0f;
    motionCommandLock->bodyYVel = 0.0f;
    motionCommandLock->bodyWVel = 0.0f;
    motionCommandLock->dribbler = 0;

    auto radioErrorLock = radioError.unsafe_value();
    radioErrorLock->isValid = false;
    radioErrorLock->lastUpdate = 0;
    radioErrorLock->hasError = false;

    printf("INFO: Radio initialized\r\n");
}

void RadioModule::entry(void) {
    {
        auto batteryVoltageLock = batteryVoltage.lock();
        auto fpgaStatusLock = fpgaStatus.lock();
        auto robotIDLock = robotID.lock();
        auto kickerInfoLock = kickerInfo.lock();
        // Just check to see if our robot id is valid
        // That way we don't conflict with other robots on the network
        // that are working
        if (batteryVoltageLock->isValid && fpgaStatusLock->isValid && robotIDLock->isValid) {
            link.send(batteryVoltageLock.value(), fpgaStatusLock.value(), kickerInfoLock.value(), robotIDLock.value());
        }
    }

    {
        auto kickerCommandLock = kickerCommand.lock();
        auto motionCommandLock = motionCommand.lock();
        auto radioErrorLock = radioError.lock();

        // Try read
        // Clear buffer of old packets such that we can get the lastest packet
        // If you don't do this there is a significant lag of 300ms or more
        while (link.receive(kickerCommandLock.value(), motionCommandLock.value())) {
            kickerCommandLock->isValid = true;
            kickerCommandLock->lastUpdate = HAL_GetTick();

            motionCommandLock->isValid = true;
            motionCommandLock->lastUpdate = HAL_GetTick();
        }

        radioErrorLock->isValid = true;
        radioErrorLock->lastUpdate = HAL_GetTick();
        radioErrorLock->hasError = false;
    }
}