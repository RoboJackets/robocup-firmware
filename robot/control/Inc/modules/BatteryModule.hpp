#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 

#include "drivers/Battery.hpp"

class BatteryModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 1.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 1;

    BatteryModule(LockedStruct<BatteryVoltage>& batteryVoltage);

    virtual void entry(void);

private:
    LockedStruct<BatteryVoltage>& batteryVoltage;
    
    Battery battery;
};