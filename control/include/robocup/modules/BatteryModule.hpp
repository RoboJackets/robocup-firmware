#pragma once

#include "GenericModule.hpp"
#include <memory>
#include "drivers/Battery.hpp"

/**
 * Module interfacing with battery and handling battery status
 */
class BatteryModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that BatteryModule should run (Hz).
     */
    static constexpr float kFrequency = 1.0f;

    /**
     * Number of seconds elapsed (period) between BatteryModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     *
     * Uses the default priority of 1.
     */
    static constexpr int kPriority = 1;

    /**
     * Constructor for BatteryModule
     *
     * @param batteryVoltage Shared memory location containing data on battery voltage and critical status
     */
    explicit BatteryModule(LockedStruct<BatteryVoltage>& batteryVoltage);

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Updates `batteryVoltage` with new information from `battery`
     */
    void entry() override;

private:
    LockedStruct<BatteryVoltage>& batteryVoltage;

    Battery battery;
};
