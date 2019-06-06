#include "modules/LEDModule.hpp"
#include "mtrain.hpp"

LEDModule::LEDModule(BatteryVoltage *const batteryVoltage,
                     FPGAStatus *const fpgaStatus,
                     RadioError *const radioError)
    : batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      radioError(radioError), leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false)  /**, all leds **/ {
}

void LEDModule::entry(void) {
    // update battery, fpga, and radio status leds
}

void LEDModule::fpgaInitialized() {
    // neo pixel stuff
}

void LEDModule::radioInitialized() {
    // neo pixel stuff
}

void LEDModule::kickerInitialized() {
    // neo pixel stuff
}

void LEDModule::fullyInitialized() {
    // green neo pixel
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