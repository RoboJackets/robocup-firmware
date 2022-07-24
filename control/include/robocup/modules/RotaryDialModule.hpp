#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MCP23017.hpp"
#include "drivers/RotarySelector.hpp"
#include "drivers/IOExpanderDigitalInOut.hpp"

/**
 * Module interfacing with Rotary Dial to handle Robot IDs
 */
class RotaryDialModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that RotaryDialModule should run (Hz)
     */
    static constexpr float kFrequency = 1.0f;

    /**
     * Number of seconds elapsed (period) between RotaryDialModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for RotaryDialModule
     * @param ioExpander shared_ptr with mutex locks for MCP23017 driver
     * @param robotID Shared memory location containing ID selected for Robot on rotary dial
     */
    RotaryDialModule(LockedStruct<MCP23017>& ioExpander, LockedStruct<RobotID>& robotID);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Updates RobotID based on rotary dial
     */
    void entry() override;

private:
    LockedStruct<RobotID>& robotID;

    RotarySelector<IOExpanderDigitalInOut> dial;

    int last_robot_id = -1;
};