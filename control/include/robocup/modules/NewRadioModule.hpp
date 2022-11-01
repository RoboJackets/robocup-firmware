#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 

#include "UART.hpp"

/**
 * Module interfacing with the new radio
 */
class NewRadioModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that NewRadioModule should run (Hz).
     */
    static constexpr float kFrequency = 1.0f;

    /**
     * Number of seconds elapsed (period) between NewRadioModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     *
     * Uses the default priority of 1.
     */
    static constexpr int kPriority = 1;

    /**
     * Constructor for NewRadioModule
     *
     */
    NewRadioModule(LockedStruct<BatteryVoltage>& batteryVoltage);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     * 
     */
    void entry() override;

private:
    LockedStruct<BatteryVoltage>& batteryVoltage;
    UART uart;
    uint8_t dataBuff = -1;
};