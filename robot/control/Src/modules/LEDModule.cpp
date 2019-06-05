#include "modules/LEDModule.hpp"

LEDModule::LEDModule(BatteryVoltage *const batteryVoltage,
                     FPGAStatus *const fpgaStatus,
                     RadioError *const radioError)
    : batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      radioError(radioError) /**, all leds **/ {
    // todo init leds
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
    // toggle all together
    // state var != state var
}

void LEDModule::missedModuleRun() {
    // toggle alternating
    // state var != state var
}