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