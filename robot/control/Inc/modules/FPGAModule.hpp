#pragma once

#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/FPGA.hpp"

#include "mtrain.hpp"
#include "SPI.hpp"

#include <memory>

class FPGAModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 100.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000000L / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 418; // us

    FPGAModule(std::shared_ptr<SPI> spi,
               MotorCommand *const motorCommand,
               FPGAStatus *const fpgaStatus,
               MotorFeedback *const motorFeedback);

    virtual void entry(void);

private:
    MotorCommand *const motorCommand;
    MotorFeedback *const motorFeedback;
    FPGAStatus *const fpgaStatus;

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
