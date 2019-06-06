#include "modules/KickerModule.hpp"
#include "mtrain.hpp"

KickerModule::KickerModule(KickerCommand *const kickerCommand,
                           KickerInfo *const kickerInfo)
    : kickerCommand(kickerCommand), kickerInfo(kickerInfo),
      prevKickTime(0), kicker() {
    // todo init kicker
    kicker.flash(true, false);

    kicker.start();

    kickerInfo->isValid = false;
    kickerInfo->lastUpdate = 0;
    kickerInfo->kickerHasError = false;
    kickerInfo->kickerCharged = false;
    kickerInfo->ballSenseTriggered = false;
}

void KickerModule::entry(void) {
    // Check if valid
    // and within the last few ms
    // and not same as previous command
    if (kickerCommand->isValid &&
        static_cast<int32_t>(HAL_GetTick() - kickerCommand->lastUpdate) < 100 &&
        kickerCommand->lastUpdate != prevKickTime) {
        
        kicker.kickType(kickerCommand->shootMode == KickerCommand::ShootMode::CHIP);
        
        switch (kickerCommand->triggerMode) {
            case KickerCommand::TriggerMode::OFF:
                kicker.setChargeAllowed(false); // todo: check this
                kicker.cancelBreakbeam();
                break;

            case KickerCommand::TriggerMode::IMMEDIATE:
                kicker.setChargeAllowed(true);
                kicker.cancelBreakbeam();
                kicker.kick(kickerCommand->kickStrength);
                break;

            case KickerCommand::TriggerMode::ON_BREAK_BEAM:
                kicker.setChargeAllowed(true);
                kicker.kickOnBreakbeam(kickerCommand->kickStrength);
                break;

            case KickerCommand::TriggerMode::INVALID:
                kicker.setChargeAllowed(false);
                kicker.cancelBreakbeam();
                break;
        }
    }

    kickerInfo->isValid = true;
    kickerInfo->lastUpdate = HAL_GetTick();
    kickerInfo->kickerHasError = !kicker.isHealthy();
    kickerInfo->ballSenseTriggered = kicker.isBallSensed();
    kickerInfo->kickerCharged = kicker.getVoltage() > KickerBoard::ChargedVoltageCutoff;
}