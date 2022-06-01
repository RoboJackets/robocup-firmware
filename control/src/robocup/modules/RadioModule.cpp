#include "modules/RadioModule.hpp"
#include "iodefs.h"

RadioModule::RadioModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                         LockedStruct<FPGAStatus>& fpgaStatus,
                         LockedStruct<KickerInfo>& kickerInfo,
                         LockedStruct<RobotID>& robotID,
                         LockedStruct<KickerCommand>& kickerCommand,
                         LockedStruct<MotionCommand>& motionCommand,
                         LockedStruct<RadioError>& radioError,
                         LockedStruct<DebugInfo>& debugInfo)
    : GenericModule(kPeriod, "radio", kPriority),
      batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), robotID(robotID),
      kickerCommand(kickerCommand), motionCommand(motionCommand),
      radioError(radioError),
      debugInfo(debugInfo), link(),
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
    radioErrorLock->hasConnectionError = false;
    radioErrorLock->hasSoccerConnectionError = false;
}

void RadioModule::start() {
    link.init();
    printf("[INFO] Radio initialized\r\n");
    radioError.lock()->initialized = link.isRadioInitialized();
}

void RadioModule::entry() {
    BatteryVoltage battery;
    FPGAStatus fpga;
    RobotID id;
    KickerInfo kicker;
    DebugInfo debug;

    {
        battery = batteryVoltage.lock().value();
        fpga = fpgaStatus.lock().value();
        id = robotID.lock().value();
        kicker = kickerInfo.lock().value();
        std::swap(debug, debugInfo.lock().value());
    }

    // Just check to see if our robot id is valid
    // That way we don't conflict with other robots on the network
    // that are working
    if (battery.isValid && fpga.isValid && id.isValid) {
        link.send(battery, fpga, kicker, id, debug);
        printf("\x1B[32m [INFO] Radio sent information \x1B[37m\r\n");
    }

    {
        MotionCommand received_motion_command;
        KickerCommand received_kicker_command;

        // Try read
        // Clear buffer of old packets such that we can get the lastest packet
        // If you don't do this there is a significant lag of 300ms or more
        while (link.receive(received_kicker_command, received_motion_command)) {
        }

        // link.receive(received_kicker_command, received_motion_command);
        printf("\x1B[32m [INFO] Radio probably received information \x1B[37m \r\n");

        if (received_motion_command.isValid) {
            motionCommand.lock().value() = received_motion_command;
        }

        if (received_kicker_command.isValid) {
            kickerCommand.lock().value() = received_kicker_command;
        }

        {
            auto radioErrorLock = radioError.lock();
            radioErrorLock->isValid = true;
            radioErrorLock->lastUpdate = HAL_GetTick();
            radioErrorLock->hasConnectionError = link.isRadioConnected();
            radioErrorLock->hasSoccerConnectionError = link.hasSoccerTimedOut();
        }
    }
}
