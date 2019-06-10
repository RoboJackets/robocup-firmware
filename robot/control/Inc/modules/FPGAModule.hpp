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
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 0; // ms

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
};