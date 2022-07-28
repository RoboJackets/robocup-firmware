#pragma once

#include "mtrain.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/FPGA.hpp"

#include "SPI.hpp"

#include <memory>
#include <LockedStruct.hpp>

/**
 * Module interfacing with FPGA and handling FPGA status
 */
class FPGAModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that FPGAModule should run (Hz)
     */
    static constexpr float kFrequency = 250.0f;

    /**
     * Number of seconds elapsed (period) between FPGAModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
    * Constructor for FPGAModule
    * @param spi Pointer to SPI object which handles communication on SPI bus
    * @param motorCommand Shared memory location containing wheel motor duty cycles and dribbler rotation speed
    * @param fpgaStatus Shared memory location containing whether motors or FPGA have errors
    * @param motorFeedback Shared memory location containing encoder counts and currents to each wheel motor
    */
    FPGAModule(std::unique_ptr<SPI> spi,
               LockedStruct<MotorCommand>& motorCommand,
               LockedStruct<FPGAStatus>& fpgaStatus,
               LockedStruct<MotorFeedback>& motorFeedback);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Updates motor voltages and feedback (currents/encoder counts)
     */
    void entry() override;

private:
    LockedStruct<MotorCommand>& motorCommand;
    LockedStruct<MotorFeedback>& motorFeedback;
    LockedStruct<FPGAStatus>& fpgaStatus;

    FPGA fpga;
    bool fpgaInitialized;

    /**
     * Max amount of time that can elapse from the latest
     * command from motion control
     *
     * This is a safety feature to prevent the motors from moving
     * if the motion control dies
     */
    static constexpr uint32_t COMMAND_TIMEOUT = 250; // ms

    /**
     * Number of enc ticks per revolution of the wheel
     * Encoder is on the motor before the gear ratio
     */
    static constexpr uint32_t GEAR_RATIO = 3;
    static constexpr uint32_t ENC_TICK_PER_REV = 2048 * GEAR_RATIO;
};
