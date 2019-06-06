#pragma once

#include "GenericModule.hpp"
#include "MicroPackets.hpp" 

#include "drivers/Battery.hpp"

class BatteryModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 1.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 0; // ms

    BatteryModule(BatteryVoltage *const batteryVoltage);

    virtual void entry(void);

private:
    BatteryVoltage *const batteryVoltage;
    
    Battery battery;
};