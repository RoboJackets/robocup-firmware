#pragma once

#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/FPGA.hpp"

#include "mtrain.hpp"
#include "SPI.hpp"

#include <memory>
#include <LockedStruct.hpp>

class FPGAModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 200.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 2;

    FPGAModule(std::unique_ptr<SPI> spi,
               LockedStruct<MotorCommand>& motorCommand,
               LockedStruct<FPGAStatus>& fpgaStatus,
               LockedStruct<MotorFeedback>& motorFeedback);

    void start() override;

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
