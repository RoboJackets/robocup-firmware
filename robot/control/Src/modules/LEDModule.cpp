#include "modules/LEDModule.hpp"
#include "iodefs.h"

LEDModule::LEDModule(std::shared_ptr<MCP23017> ioExpander,
                     BatteryVoltage *const batteryVoltage,
                     FPGAStatus *const fpgaStatus,
                     RadioError *const radioError)
    : batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      radioError(radioError), ioExpander(ioExpander),
      leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false) {

    ioExpander->writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask), IOExpanderErrorLEDMask);
}

void LEDModule::entry(void) {
    // update battery, fpga, and radio status leds
    uint16_t errors = 0;
    int motors[5] = {ERR_LED_M1, ERR_LED_M2, ERR_LED_M3, ERR_LED_M4, ERR_LED_DRIB};

    if (!fpgaStatus->isValid || fpgaStatus->FPGAHasError) {
        for (int i = 0; i < 5; i++) {
            errors |= (1 << motors[i]);
        }
    } else { 
        for (int i = 0; i < 5; i++) {
            errors |= (fpgaStatus->motorHasErrors[i] << motors[i]);
        }
    }

    if (!radioError->isValid || radioError->hasError) {
        errors |= (1 << ERR_LED_RADIO);
    }

    ioExpander->writeMask(~errors, IOExpanderErrorLEDMask);
    printf("%4x \r\n", errors);
}

void LEDModule::fpgaInitialized() {
    // neo pixel stuff
    leds[0] = 1;
}

void LEDModule::radioInitialized() {
    // neo pixel stuff
    leds[1] = 1;
}

void LEDModule::kickerInitialized() {
    // neo pixel stuff
    leds[2] = 1;
}

void LEDModule::fullyInitialized() {
    // green neo pixel
    leds[3] = 1;
}

void LEDModule::missedSuperLoop() {
    for (int i = 0; i < 4; i++) {
        leds[i] = missedSuperLoopToggle;
    }

    missedSuperLoopToggle = !missedSuperLoopToggle;
}

void LEDModule::missedModuleRun() {
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            leds[i] = missedModuleRunToggle;
        } else {
            leds[i] = !missedSuperLoopToggle;
        }
    }

    missedModuleRunToggle = !missedModuleRunToggle;
}