#pragma once

#include "GenericModule.hpp"
#include "mtrain.hpp"
#include "MicroPackets.hpp" 
#include "drivers/RotarySelector.hpp"

class RotaryDialModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 1.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 0; // ms

    RotaryDialModule(RobotID *const robotID);

    virtual void entry(void);

private:
    RobotID *const robotID;
    
    //RotarySelector<DigitalIn> dial;
};