#include "modules/KickerModule.hpp"

#include "mtrain.hpp"
#include "iodefs.h"
#include "delay.h"
#include <cstdio>

int Kickercount; 
 uint64_t KickerstartTime;
 
KickerModule::KickerModule(LockedStruct<SPI>& spi,
                           LockedStruct<KickerCommand>& kickerCommand,
                           LockedStruct<KickerInfo>& kickerInfo)
    : GenericModule(kPeriod, "kicker", kPriority),
      kickerCommand(kickerCommand), kickerInfo(kickerInfo),
      prevKickTime(0), nCs(std::make_shared<DigitalOut>(KICKER_CS)), kicker(spi, nCs, KICKER_RST) {
    auto kickerInfoLock = kickerInfo.unsafe_value();
    kickerInfoLock->isValid = false;
    kickerInfoLock->lastUpdate = 0;
    kickerInfoLock->kickerHasError = false;
    kickerInfoLock->kickerCharged = false;
    kickerInfoLock->ballSenseTriggered = false;
}

void KickerModule::start() {
    bool initialized = kicker.flash(false, true);
    printf("[INFO] Kicker initialized\r\n");
    {
        kickerInfo.lock()->initialized = initialized;
    }
}

void KickerModule::entry(void) {
    KickerstartTime =  DWT_GetTick();
    kicker.setChargeAllowed(true);
    // Check if valid
    // and within the last few ms
    // and not same as previous command
    {
        auto kickerCommandLock = kickerCommand.lock();
        if (kickerCommandLock->isValid) {

            kickerCommandLock->isValid = false;

            kicker.kickType(kickerCommandLock->shootMode == KickerCommand::ShootMode::KICK);

            kicker.setChargeAllowed(true);

            switch (kickerCommandLock->triggerMode) {
                case KickerCommand::TriggerMode::OFF:
                    kicker.setChargeAllowed(true);
                    kicker.cancelBreakbeam();
                    break;

                case KickerCommand::TriggerMode::IMMEDIATE:
                    kicker.setChargeAllowed(true);
                    kicker.kick(kickerCommandLock->kickStrength);
                    break;

                case KickerCommand::TriggerMode::ON_BREAK_BEAM:
                    kicker.setChargeAllowed(true);
                    kicker.kickOnBreakbeam(kickerCommandLock->kickStrength);
                    break;

                case KickerCommand::TriggerMode::INVALID:
                    kicker.setChargeAllowed(false);
                    kicker.cancelBreakbeam();
                    break;
            }
        }
    }

    // Do all the commands
    kicker.service();

    {
        auto kickerInfoLock = kickerInfo.lock();
        kickerInfoLock->isValid = true;
        kickerInfoLock->lastUpdate = HAL_GetTick();
        kickerInfoLock->kickerHasError = kicker.isHealthy();
        kickerInfoLock->ballSenseTriggered = kicker.isBallSensed();
        kickerInfoLock->kickerCharged = kicker.isCharged();
    }
    if (Kickercount % 50==0) {
        printf("Kicker Time Elapsed: %f\r\n", ((DWT_GetTick()) - KickerstartTime) / 216.0f);
       
        
    }
    Kickercount++;
}
