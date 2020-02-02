#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MCP23017.hpp"
#include "drivers/RotarySelector.hpp"
#include "drivers/IOExpanderDigitalInOut.hpp"

class RotaryDialModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 1.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 3;

    RotaryDialModule(LockedStruct<MCP23017>& ioExpander, LockedStruct<RobotID>& robotID);

    void start() override;
    void entry() override;

private:
    LockedStruct<RobotID>& robotID;
    
    RotarySelector<IOExpanderDigitalInOut> dial;
};