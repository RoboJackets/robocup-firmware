#include "modules/LEDModule.hpp"
#include "iodefs.h"

bool operator==(const Error& e1, const Error& e2) {
    return e1.led0 == e2.led0 && e1.led1 == e2.led1 && e1.led2 == e2.led2;
}

LEDModule::LEDModule(LockedStruct<MCP23017> &ioExpander, LockedStruct<SPI> &dotStarSPI,
                     LockedStruct<BatteryVoltage> &batteryVoltage, LockedStruct<FPGAStatus> &fpgaStatus,
                     LockedStruct<KickerInfo> &kickerInfo, LockedStruct<RadioError> &radioError,
                     LockedStruct<IMUData> &imuData, LockedStruct<LEDCommand> &ledCommand)
    : GenericModule(kPeriod, "led", kPriority),
      batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), radioError(radioError),
      imuData(imuData),
      ioExpander(ioExpander),
      dotStarSPI(dotStarSPI),
      ledCommand(ledCommand),
      dotStarNCS(DOT_STAR_CS),
      leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false) {
    auto ioExpanderLock = ioExpander.lock();
    ioExpanderLock->init();
    ioExpanderLock->config(0x00FF, 0x00FF, 0x00FF);
    dotStarNCS.write(true);
}

void LEDModule::start() {
    auto ioExpanderLock = ioExpander.lock();
    setColor(Color::WHITE, Color::WHITE, Color::WHITE);
    errToggles.fill(false);
    index = 0;
    ioExpanderLock->writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask), IOExpanderErrorLEDMask);
    middle_led_color = Color::YELLOW;
    setColor(Color::WHITE, middle_led_color, Color::WHITE);
}

extern std::vector<const char*> failed_modules;
extern size_t free_space;

void LEDModule::entry() {
    auto led_command = ledCommand.lock().value();

    // toggle middle led color
    middle_led_color =  middle_led_color == Color::BLACK ? Color::YELLOW : Color::BLACK;

    Color role_color;
    auto role = led_command.role;
    Color connection_status = Color::GREEN;
    if (!led_command.isValid) {
        role_color = Color::BLACK;
        connection_status = Color::DARKRED;
    } else {
        switch (role) {
            case 0:
                role_color = Color::WHITE;
                break;
            case 1:
                role_color = Color::BLUE;
                break;
            case 2:
                role_color = Color::MAUVE;
                break;
            default:
                role_color = Color::BLACK;
                break;
        }
    }
    setColor(connection_status, middle_led_color, role_color);
}

void LEDModule::fpgaInitialized() { leds[0] = 1; }

void LEDModule::radioInitialized() { leds[1] = 1; }

void LEDModule::kickerInitialized() { leds[2] = 1; }

void LEDModule::fullyInitialized() { leds[3] = 1; }

void LEDModule::missedSuperLoop() {
    for (int i = 1; i < 4; i++) {
        leds[i] = missedSuperLoopToggle;
    }

    missedSuperLoopToggle = !missedSuperLoopToggle;
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
}

void LEDModule::setColor(uint32_t led0, uint32_t led1, uint32_t led2) {
    // 0 - 31
    uint8_t brightness = 2 | 0xE0;

    std::vector<uint8_t> data = {0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 brightness,
                                 (led0 >> 0) & 0xFF,
                                 (led0 >> 8) & 0xFF,
                                 (led0 >> 16) & 0xFF,
                                 brightness,
                                 (led1 >> 0) & 0xFF,
                                 (led1 >> 8) & 0xFF,
                                 (led1 >> 16) & 0xFF,
                                 brightness,
                                 (led2 >> 0) & 0xFF,
                                 (led2 >> 8) & 0xFF,
                                 (led2 >> 16) & 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF,
                                 0xFF};
    {
        auto dotStarSPILock = dotStarSPI.lock();
        dotStarSPILock->frequency(200'000);
        dotStarNCS.write(false);
        dotStarSPILock->transmitReceive(data);
        dotStarNCS.write(true);
    }
}

void LEDModule::displayErrors() {
    // If there are no errors, then there is no point in displaying anything
    if (std::none_of(errToggles.begin(), errToggles.end(), [](bool b) { return b; })) {
        setColor(0x000000, 0x000000, 0x000000);
    } else if (lightsOn && framesOnCounter >= framesOn){
        // If error lights have been on long enough, switch them off
        framesOnCounter = 0;
        lightsOn = false;
        setColor(0x000000, 0x000000, 0x000000);
    } else if (!lightsOn && framesOffCounter >= framesOff) {
        // If error lights have been off long enough, switch them back on to next error role_color
        framesOffCounter = 0;
        lightsOn = true;

        // Make sure index loops back if it has reached end of vector
        // Since we know there is at least one error, we can safely use a while loop to find it.
        do {
            index = (index + 1) % errToggles.size();
        }
        while(!errToggles.at(index));

        Error e = ERR_LIST.at(index);
        setColor(e.led0, e.led1, e.led2);
    } else {
        // Increment frame counters for lights on/off
        lightsOn ? framesOnCounter++ : framesOffCounter++;
    }
}

void LEDModule::setError(const Error e, bool toggle) {
    const Error *error = std::find(ERR_LIST.begin(), ERR_LIST.end(), e);
    if (error == ERR_LIST.end()) {
        printf("[ERROR] Invalid error code\r\n");
    } else {
        errToggles.at(std::distance(ERR_LIST.begin(), error)) = toggle;
    }
}
