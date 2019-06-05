#include "modules/KickerModule.hpp"

KickerModule::KickerModule(KickerCommand *const kickerCommand,
                           KickerInfo *const kickerInfo)
    : kickerCommand(kickerCommand), kickerInfo(kickerInfo)
    /**,kicker()**/ {
    // todo init kicker
    //kicker.flash(true, false);

    //kicker.start();

    kickerInfo->isValid = false;
    kickerInfo->lastUpdate = 0;
}

void KickerModule::entry(void) {
    // todo invalidate command after?
    if (kickerCommand->isValid) {
        //kicker.kick(strength);
        //kicker.kickOnBreakbeam();
        //kicker.cancelBreakbeam();
        //kicker.setChargeAllowed(bool);
    }

    kickerInfo->isValid = true;
    kickerInfo->lastUpdate = HAL_GetTick();
    // kicker.isBallSensed();
    // kicker.isCharging();
    // kicker.getVoltage();
    // kicker.isHealth();
}