#include "modules/FPGAModule.hpp"
#include "iodefs.h"

FPGAModule::FPGAModule(std::shared_ptr<SPI> spi,
                       MotorCommand *const motorCommand,
                       FPGAStatus *const fpgaStatus,
                       MotorFeedback *const motorFeedback) 
    : motorCommand(motorCommand), motorFeedback(motorFeedback),
      fpgaStatus(fpgaStatus),
      fpga(spi, FPGA_CS, FPGA_INIT, FPGA_PROG, FPGA_DONE),
      fpgaInitialized(false) {

    fpgaInitialized = fpga.configure();

    if (fpgaInitialized) {
        printf("INFO: FPGA configured\r\n");
    } else {
        printf("WARN: FPGA could not be configured\r\n");
    }

    motorFeedback->isValid = false;
    motorFeedback->lastUpdate = 0;
    for (int i = 0; i < 4; i++) {
        motorFeedback->encoders[i] = 0.0f;
        motorFeedback->currents[i] = 0.0f;
    }

    fpgaStatus->isValid = false;
    fpgaStatus->lastUpdate = 0;
    // msb is 1 to indicate no errors
    fpgaStatus->FPGAHasError = false;
    for (int i = 0; i < 4; i++) {
        fpgaStatus->motorHasErrors[i] = false;
    }
}

void FPGAModule::entry(void) {
    // FPGA not initialized
    // report error and return
    if (!fpgaInitialized) {
        // report error and return
        fpgaStatus->isValid = true;
        fpgaStatus->lastUpdate = HAL_GetTick();
        fpgaStatus->FPGAHasError = true;

        return;
    }

    // FPGA initialized so we all good
    std::array<int16_t, 5> dutyCycles{0, 0, 0, 0, 0};
    std::array<int16_t, 5> encDeltas{};

    // Make sure commands are valid
    // If they are not valid, we automatically send a 0 duty cycle
    if (motorCommand->isValid &&
        (HAL_GetTick() - motorCommand) < COMMAND_TIMEOUT) {

        for (int i = 0; i < 4; i++)
            dutyCycles.at(i) = motorCommand->wheels[i];
        dutyCycles.at(4) = motorCommand->dribbler;

    }

    // Communicate with FPGA
    uint8_t status = fpga.set_duty_get_enc(
        dutyCycles.data(), dutyCycles.size(),
        encDeltas.data(), encDeltas.size());


    // Do conversion and throw into shared mem

    motorFeedback->isValid = true;
    motorFeedback->lastUpdate = HAL_GetTick();

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
    const float dt = enc_deltas[4] * (1 / 18.432e6) * 2 * 128;

    // Convert encoders to rad/sec from enc ticks since last readin
    for (int i = 0; i < 4; i++) {
        // (rad / s) = (enc) * (rev / enc) * (rad / rev) * (1 / sec)
        motorFeedback->encoders[i] = encDeltas[i] * (1 / ENC_TICK_PER_REV) * (2*M_PI / 1) * (1 / dt);
    }

    // Convert from adc lsb to amp
    for (int i = 0; i < 4; i++) {
        motorFeedback->currents[i] = 0.0f;
    }


    fpgaStatus->isValid = true;
    fpgaStatus->lastUpdate = HAL_GetTick();
    // msb is 1 to indicate no errors
    fpgaStatus->FPGAHasError = (status & (1 << 7)) == 0;

    // 1 is to indicate error on the specific wheel
    for (int i = 0; i < 4; i++) {
        fpgaStatus->motorHasErrors[i] = (status & (1 << i)) == 1;
    }
    
    //printf("%0x %0x %0x %0x %0x %0x\r\n", status, encDeltas[0], encDeltas[1], encDeltas[2], encDeltas[3], encDeltas[4]);
}