#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 

#include "drivers/Battery.hpp"

class BatteryModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that BatteryModule should run, in Hz.
     */
    static constexpr float kFrequency = 1.0f;

    /**
     * Number of seconds elapsed (period) between BatteryModule runs, in milliseconds
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     * Uses the default priority of 1.
     */
    static constexpr int kPriority = 1;

    /**
     * Constructor for BatteryModule
     * @param Object storing information from `battery`
     */
    explicit BatteryModule(LockedStruct<BatteryVoltage>& batteryVoltage);

    /**
    * Code to run when called by RTOS
    * Updates `batteryVoltage` with new information from `battery`
    */
    void entry() override;

private:
    /**
     * Stores information from `battery`
     * Set by constructor
     */
    LockedStruct<BatteryVoltage>& batteryVoltage;

    /**
    * Interfaces with physical battery to get updates for `batteryVoltage`
    */
    Battery battery;
};