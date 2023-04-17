#pragma once

#include <LockedStruct.hpp>
#include "GenericModule.hpp"
#include "MicroPackets.hpp"

/**
 */
class ResetModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that RotaryDialModule should run (Hz)
     */
    static constexpr float kFrequency = 0.125f;

    /**
     * Number of seconds elapsed (period) between RotaryDialModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 5;

    /**
     */
    ResetModule();

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     */
    void entry() override;
};