#pragma once

#include "GenericModule.hpp"
#include "drivers/KickerBoard.hpp"
#include <cstdio>


/**
 * Module interfacing with Kicker and handling Kicker status
 */
class KickerModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that KickerModule should run (Hz)
     */
    static constexpr float kFrequency = 25.0f;

    /**
     * Number of seconds elapsed (period) between KickerModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 2;

    /**
     * Constructor for KickerModule
     * @param spi Pointer to SPI object which handles communication on SPI bus
     * @param kickerCommand Shared memory location containing kicker shoot mode, trigger mode, and kick strength
     * @param kickerInfo Shared memory location containing kicker status
     */
    KickerModule(LockedStruct<SPI>& spi,
                 LockedStruct<KickerCommand>& kickerCommand,
                 LockedStruct<KickerInfo>& kickerInfo);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Sets kicker strength, shoot mode and trigger mode from `kickerCommand`. Updates kicker status in `kickerInfo`
     */
    void entry() override;

private:
    LockedStruct<KickerCommand>& kickerCommand;
    LockedStruct<KickerInfo>& kickerInfo;

    /**
     * Time at which last command to kick was sent (milliseconds)
     *
     * Prevents double triggers on the same packet
     */
    uint32_t prevKickTime;

    /**
     * PinName for the negated chip select line
     */
    std::shared_ptr<DigitalOut> nCs;
    KickerBoard kicker;
};
