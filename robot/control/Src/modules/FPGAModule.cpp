#include "modules/FPGAModule.hpp"
#include "iodefs.h"

#include <cmath>
#include <delay.h>

using namespace std::literals;

FPGAModule::FPGAModule(std::unique_ptr<SPI> spi,
                       LockedStruct<MotorCommand>& motorCommand,
                       LockedStruct<FPGAStatus>& fpgaStatus,
                       LockedStruct<MotorFeedback>& motorFeedback)
    : GenericModule(kPeriod, "fpga", kPriority, 1 << 10),
      motorCommand(motorCommand), motorFeedback(motorFeedback),
      fpgaStatus(fpgaStatus),
      fpga(std::move(spi), FPGA_CS, FPGA_INIT, FPGA_PROG, FPGA_DONE),
      fpgaInitialized(false) {
    {
        auto motorFeedbackLock = motorFeedback.unsafe_value();
        motorFeedbackLock->isValid = false;
        motorFeedbackLock->lastUpdate = 0;
        for (int i = 0; i < 4; i++) {
            motorFeedbackLock->encoders[i] = 0.0f;
            motorFeedbackLock->currents[i] = 0.0f;
        }
    }

    {
        auto fpgaStatusLock = fpgaStatus.unsafe_value();
        fpgaStatusLock->isValid = false;
        fpgaStatusLock->lastUpdate = 0;
        // msb is 1 to indicate no errors
        fpgaStatusLock->FPGAHasError = false;
        for (int i = 0; i < 4; i++) {
            fpgaStatusLock->motorHasErrors[i] = false;
        }
    }
}

void FPGAModule::start() {
    vTaskDelay(3000);
    auto fpgaStatusLock = fpgaStatus.lock();
    fpgaInitialized = fpga.configure();
    printf("INFO: FPGA probably configured\r\n");
    fpgaStatusLock->initialized = fpgaInitialized;
}

void FPGAModule::entry() {
    // We've still got an issue in hardware: the INIT_B pin is never pulled
    // high after FPGA boot, so we never actually receive a signal. Until we
    // fix this, just assume it's been initialized properly.
    /*
    if (!fpgaInitialized) {
        // If the FPGA was never initialized, report an error and return.
        auto fpgaStatusLock = fpgaStatus.lock();
        fpgaStatusLock->isValid = true;
        fpgaStatusLock->lastUpdate = HAL_GetTick();
        fpgaStatusLock->FPGAHasError = true;

        return;
    }
    */

    // FPGA initialized so we all good
    std::array<int16_t, 5> dutyCycles{0, 0, 0, 0, 0};
    std::array<int16_t, 5> encDeltas{};

    {
        auto motorCommandLock = motorCommand.lock();
        // Make sure commands are valid
        // If they are not valid, we automatically send a 0 duty cycle
        if (motorCommandLock->isValid &&
            (HAL_GetTick() - motorCommandLock->lastUpdate) < COMMAND_TIMEOUT) {

            for (int i = 0; i < 4; i++) {
                dutyCycles.at(i) = static_cast<int16_t>(
                        motorCommandLock->wheels[i] * fpga.MAX_DUTY_CYCLE / 2);
                if (dutyCycles.at(i) > fpga.MAX_DUTY_CYCLE) {
                    dutyCycles.at(i) = fpga.MAX_DUTY_CYCLE;
                } else if (dutyCycles.at(i) < -fpga.MAX_DUTY_CYCLE) {
                    dutyCycles.at(i) = -fpga.MAX_DUTY_CYCLE;
                }
            }
            dutyCycles.at(4) = motorCommandLock->dribbler;
        }
    }

    // Communicate with FPGA
    uint8_t status = fpga.set_duty_get_enc(
        dutyCycles.data(), dutyCycles.size(),
        encDeltas.data(), encDeltas.size());

    /*
     * The time since the last update is derived with the value of
     * WATCHDOG_TIMER_CLK_WIDTH in robocup.v
     *
     * The last encoder reading (5th one) from the FPGA is the watchdog
     * timer's tick since the last SPI transfer.
     *
     * Multiply the received tick count by:
     *     (1/18.432) * 2 * (2^WATCHDOG_TIMER_CLK_WIDTH)
     *
     * This will give you the duration since the last SPI transfer in
     * microseconds (us).
     *
     * For example, if WATCHDOG_TIMER_CLK_WIDTH = 6, here's how you would
     * convert into time assuming the fpga returned a reading of 1265 ticks:
     *     time_in_us = [ 1265 * (1/18.432) * 2 * (2^6) ] = 8784.7us
     *
     * The precision would be in increments of the multiplier. For
     * this example, that is:
     *     time_precision = 6.94us
     *
     */
    float dt = static_cast<float>(encDeltas[4]) * (1 / 18.432e6) * 2 * 128;

    // Force dt to be nonzero
    // so nan's don't filter up
    // If it's too small, just assume dt is massive
    // so theres no recordered encoder outut since it's
    // probably junk data
    if (dt < 0.0001) {
        dt = 1;
    }

    {
        auto motorFeedbackLock = motorFeedback.lock();
        // Convert encoders to rad/sec from enc ticks since last reading
        for (int i = 0; i < 4; i++) {
            // (rad / s) = (enc) * (rev / enc) * (rad / rev) * (1 / sec)
            motorFeedbackLock->encoders[i] =
                    static_cast<float>(encDeltas[i]) * (1 / static_cast<float>(ENC_TICK_PER_REV)) * (2 * M_PI / 1) *
                    (1 / dt);
        }

        // Convert from adc lsb to amp
        for (int i = 0; i < 4; i++) {
            motorFeedbackLock->currents[i] = 0.0f;
        }

        motorFeedbackLock->isValid = true;
        motorFeedbackLock->lastUpdate = HAL_GetTick();
    }

    {
        auto fpgaStatusLock = fpgaStatus.lock();
        fpgaStatusLock->isValid = true;
        fpgaStatusLock->lastUpdate = HAL_GetTick();
        // msb is 1 to indicate no errors
        fpgaStatusLock->FPGAHasError = (status & (1 << 7)) == 0;

        // 1 is to indicate error on the specific wheel
        for (int i = 0; i < 4; i++) {
            fpgaStatusLock->motorHasErrors[i] = (status & (1 << i)) == 1;
        }
    }
}
