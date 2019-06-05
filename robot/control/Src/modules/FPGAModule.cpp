#include "modules/FPGAModule.hpp"

FPGAModule::FPGAModule(MotorCommand *const motorCommand,
                       FPGAStatus *const fpgaStatus,
                       MotorFeedback *const motorFeedback) 
    : motorCommand(motorCommand), fpgaStatus(fpgaStatus),
      motorFeedback(motorFeedback) /**, fpga()**/{

    // todo configure fpga using new header
    if (fpga.configure("")) {
        // we good
    } else {
        // rip
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
    fpgaStatus->FPGAHasError = true;
    for (int i = 0; i < 4; i++) {
        fpgaStatus->motorHasErrors[i] = false;
    }
}

void FPGAModule::entry(void) {
    std::array<uint16_t, 5> dutyCycles{0, 0, 0, 0, 0};
    std::array<uint16_t, 5> encDeltas{};

    // Make sure commands are valid
    // todo: check timeout
    // todo: 0 motor if errored
    if (motorCommand->isValid) {
        for (int i = 0; i < 4; i++)
            dutyCycles.at(i) = motorCommand->wheels[i];
        dutyCycles.at(4) = motorCommand->dribbler;
    }

    uint8_t status = fpga.set_duty_get_enc(
        dutyCycles.data(), dutyCycles.size(),
        encDeltas.data(), encDeltas.size());

    // todo dt calc
    // todo gear ratio enc tick calc

    motorFeedback->isValid = true;
    motorFeedback->lastUpdate = HAL_GetTick();
    for (int i = 0; i < 4; i++) {
        motorFeedback->encoders[i] = 0.0f;
        motorFeedback->currents[i] = 0.0f;
    }

    fpgaStatus->isValid = true;
    fpgaStatus->lastUpdate = HAL_GetTick();
    // msb is 1 to indicate no errors
    fpgaStatus->FPGAHasError = status & (1 << 7) == 0;
    for (int i = 0; i < 4; i++) {
        fpgaStatus->motorHasErrors[i] = status & (1 << i) == 1;
    }
}