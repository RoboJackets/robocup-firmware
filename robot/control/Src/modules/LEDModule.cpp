#include "modules/LEDModule.hpp"
#include "iodefs.h"

LEDModule::LEDModule(std::shared_ptr<MCP23017> ioExpander,
                     BatteryVoltage *const batteryVoltage,
                     FPGAStatus *const fpgaStatus,
                     KickerInfo *const kickerInfo,
                     RadioError *const radioError)
    : batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), radioError(radioError),
      dot_star_spi(DOT_STAR_SPI_BUS, std::nullopt, 1'000'000), ioExpander(ioExpander),
      leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false) {

    setColor(0xFFFFFF, 0xFFFFFF);

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

        setColor(0x0000FF, 0xFFFFFF);
    } else { 
        for (int i = 0; i < 5; i++) {
            errors |= (fpgaStatus->motorHasErrors[i] << motors[i]);
        }
    }

    if (!radioError->isValid || radioError->hasError) {
        errors |= (1 << ERR_LED_RADIO);
    }

    if (kickerInfo->isValid || kickerInfo->kickerHasError) {
        errors |= (1 << ERR_LED_KICK);
    }

    ioExpander->writeMask(errors, IOExpanderErrorLEDMask);
    leds[0].toggle();
}

void LEDModule::fpgaInitialized() {
    // neo pixel stuff
    setColor(0xFFFF00, 0xFFFFFF);
    leds[0] = 1;
}

void LEDModule::radioInitialized() {
    // neo pixel stuff
    setColor(0xFF00FF, 0xFFFFFF);
    leds[1] = 1;
}

void LEDModule::kickerInitialized() {
    // neo pixel stuff
    setColor(0x00FFFF, 0xFFFFFF);
    leds[2] = 1;
}

void LEDModule::fullyInitialized() {
    // green neo pixel
    setColor(0x00FF00, 0x00FF00);
    leds[3] = 1;
}

void LEDModule::missedSuperLoop() {
    for (int i = 0; i < 4; i++) {
        leds[i] = missedSuperLoopToggle;
    }

    missedSuperLoopToggle = !missedSuperLoopToggle;

    // Orange
    setColor(0x3376DC, 0xFFFFFF);
}

void LEDModule::missedModuleRun() {
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            leds[i] = (int)missedModuleRunToggle;
        } else {
            leds[i] = (int)!missedSuperLoopToggle;
        }
    }

    missedModuleRunToggle = !missedModuleRunToggle;

    // Yellow
    setColor(0x3FD0F4, 0xFFFFFF);
}

void LEDModule::setColor(uint32_t led0, uint32_t led1) {
    // 0 - 31
    uint8_t brightness = 2 | 0xE0;

    std::vector<uint8_t> data;

    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    data.push_back(brightness);
    data.push_back((led0 >> 16) & 0xFF);
    data.push_back((led0 >> 8) & 0xFF);
    data.push_back((led0 >> 0) & 0xFF);
    
    data.push_back(brightness);
    data.push_back((led1 >> 16) & 0xFF);
    data.push_back((led1 >> 8) & 0xFF);
    data.push_back((led1 >> 0) & 0xFF);
    
    data.push_back(0xFF);
    data.push_back(0xFF);
    data.push_back(0xFF);
    data.push_back(0xFF);

    dot_star_spi.transmit(data);
}