#pragma once

#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MCP23017.hpp"
#include "drivers/RotarySelector.hpp"
#include "drivers/IOExpanderDigitalInOut.hpp"

class RotaryDialModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 1.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000000L / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 10; // us

    RotaryDialModule(std::shared_ptr<MCP23017> ioExpander, RobotID *const robotID);

    virtual void entry(void);

private:
    RobotID *const robotID;
    
    RotarySelector<IOExpanderDigitalInOut> dial;
};