#pragma once

#include "LockedStruct.hpp"
#include "GenericModule.hpp"
#include "DigitalOut.hpp"
#include "MicroPackets.hpp" 
#include "drivers/KickerBoard.hpp"

class KickerModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 25.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 2;

    KickerModule(LockedStruct<SPI>& spi,
                 LockedStruct<KickerCommand>& kickerCommand,
                 LockedStruct<KickerInfo>& kickerInfo);

    virtual void entry(void);

private:
    LockedStruct<KickerCommand>& kickerCommand;
    LockedStruct<KickerInfo>& kickerInfo;
    
    // Time of last command to kick
    // Stops double triggers on the same packet
    uint32_t prevKickTime;
    std::shared_ptr<DigitalOut> nCs;
    KickerBoard kicker;
};