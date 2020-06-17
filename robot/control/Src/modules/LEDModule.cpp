#include "modules/LEDModule.hpp"
#include "iodefs.h"

LEDModule::LEDModule(LockedStruct<MCP23017>& ioExpander,
                     LockedStruct<SPI>& sharedSPI,
                     LockedStruct<BatteryVoltage>& batteryVoltage,
                     LockedStruct<FPGAStatus>& fpgaStatus,
                     LockedStruct<KickerInfo>& kickerInfo,
                     LockedStruct<RadioError>& radioError)
    : GenericModule(kPeriod, "led", kPriority),
      batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), radioError(radioError),
      ioExpander(ioExpander),
      dotStarSPI(sharedSPI),
      dotStarNCS(DOT_STAR_CS),
      leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false) {
    dotStarNCS.write(true);
}

void LEDModule::start() {
    auto ioExpanderLock = ioExpander.lock();
    ioExpanderLock->init();
    setColor(0xFFFFFF, 0xFFFFFF);
    index = 0;
    ioExpanderLock->config(0x00FF, 0x00FF, 0x00FF);
    ioExpanderLock->writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask), IOExpanderErrorLEDMask);
}

extern std::vector<const char*> failed_modules;
extern size_t free_space;

void LEDModule::entry() {
    for (const char* c : failed_modules) {
        printf("[ERROR] Module failed to initialize: %s (initial heap size: %d)\r\n", c, free_space);
    }
    // update battery, fpga, and radio status leds
    uint16_t errors = 0;
    int motors[5] = {ERR_LED_M1, ERR_LED_M2, ERR_LED_M3, ERR_LED_M4, ERR_LED_DRIB};

    {
        auto fpgaLock = fpgaStatus.lock();

        if (fpgaLock->initialized) {
            fpgaInitialized();
        }

        if (!fpgaLock->isValid || fpgaLock->FPGAHasError) {
            for (int i = 0; i < 5; i++) {
                errors |= (1 << motors[i]);
            }

            setColor(0x0000FF, 0xFFFFFF);
        } else {
            for (int i = 0; i < 5; i++) {
                errors |= (fpgaLock->motorHasErrors[i] << motors[i]);
            }
        }
    }

    {
        auto radioLock = radioError.lock();

        if (radioLock->initialized) {
            radioInitialized();
        }

        if (!radioLock->isValid || radioLock->hasError) {
            errors |= (1 << ERR_LED_RADIO);
        }
    }

    {
        auto kickerLock = kickerInfo.lock();

        if (kickerLock->initialized) {
            radioInitialized();
        }

        if (kickerLock->isValid || kickerLock->kickerHasError) {
            errors |= (1 << ERR_LED_KICK);
        }
    }

    //ioExpander->writeMask(errors, IOExpanderErrorLEDMask);
//    leds[0].toggle();
    static bool state = true;
    state = !state;
    leds[0].write(state);

    displayErrors();
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
    for (int i = 1; i < 4; i++) {
        leds[i] = missedSuperLoopToggle;
    }

    missedSuperLoopToggle = !missedSuperLoopToggle;

    // Orange
    setColor(0x3376DC, 0xFFFFFF);
}

void LEDModule::missedModuleRun() {
    for (int i = 1; i < 4; i++) {
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

    dotStarNCS.write(false);
    dotStarSPI.lock()->transmit(data);
    dotStarNCS.write(true);
}

void displayErrors() {
    // Check if there are errors to display, else exit
    if (colorQueue.size() == 0) {
        return;
    }

    if (lightsOn && framesOn == framesOnCounter){
        // If error lights have been on long enough, switch them off
        framesOnCounter = 0;
        lightsOn = false;
        setColor(0x000000,0x000000);
    } else if (lightsOff && framesOff == framesOffCounter)
        // If error lights have been off long enough, switch them back on to next error color
        framesOffCounter = 0;
        lightsOn = true;

        // Make sure index loops back if it has reached end of vector
        if (++index > colorQueue.size()-1) {
            index = 0;
        }
        std::array<uint32_t,2> colors = colorQueue.at(index);
        setColor(colors.at(0), colors.at(1));
    } else {
        // Increment frame counters for lights on/off
        lightsOn ? framesOnCounter++ : framesOffCounter++;
    }
}

void addError(uint32_t led0, uint32_t led1) {
    std::array<uint32_t, 2> errorColors = {led0, led1};
    colorQueue.pushBack(errorColors);
}