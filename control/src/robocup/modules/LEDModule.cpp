#include "modules/LEDModule.hpp"
#include "iodefs.h"
#include "drivers/Battery.hpp"

bool operator==(const Error &e1, const Error &e2)
{
    return e1.led0 == e2.led0 && e1.led1 == e2.led1 && e1.led2 == e2.led2;
}

LEDModule::LEDModule(LockedStruct<MCP23017> &ioExpander,
                     LockedStruct<SPI> &sharedSPI,
                     LockedStruct<BatteryVoltage> &batteryVoltage,
                     LockedStruct<FPGAStatus> &fpgaStatus,
                     LockedStruct<KickerInfo> &kickerInfo,
                     LockedStruct<RadioError> &radioError,
                     LockedStruct<IMUData> &imuData)
    : GenericModule(kPeriod, "led", kPriority),
      batteryVoltage(batteryVoltage), fpgaStatus(fpgaStatus),
      kickerInfo(kickerInfo), radioError(radioError),
      imuData(imuData),
      ioExpander(ioExpander),
      dotStarSPI(sharedSPI),
      dotStarNCS(DOT_STAR_CS),
      leds({LED1, LED2, LED3, LED4}),
      missedSuperLoopToggle(false), missedModuleRunToggle(false)
{
    dotStarNCS.write(true);
}

void LEDModule::start()
{
    auto ioExpanderLock = ioExpander.lock();
    ioExpanderLock->init();
    setColor(0xFFFFFF, 0xFFFFFF, 0xFFFFFF);
    errToggles.fill(false);
    index = 0;
    ioExpanderLock->config(0x00FF, 0x00FF, 0x00FF);
    ioExpanderLock->writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask), IOExpanderErrorLEDMask);
}

extern std::vector<const char *> failed_modules;
extern size_t free_space;

void LEDModule::entry()
{
    for (const char *c : failed_modules)
    {
        printf("[ERROR] Module failed to initialize: %s (initial heap size: %d)\r\n", c, free_space);
    }
    // update battery, fpga, and radio status leds
    uint16_t errors = 0;
    int motors[5] = {ERR_LED_M1, ERR_LED_M2, ERR_LED_M3, ERR_LED_M4, ERR_LED_DRIB};

    {
        auto fpgaLock = fpgaStatus.lock();

        if (fpgaLock->initialized)
        {
            fpgaInitialized();
            setError(ERR_FPGA_BOOT_FAIL, false);
        }
        else
        {
            setError(ERR_FPGA_BOOT_FAIL, true);
        }

        if (!fpgaLock->isValid || fpgaLock->FPGAHasError)
        {
            for (int i = 0; i < 5; i++)
            {
                errors |= (1 << motors[i]);
            }
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                errors |= (fpgaLock->motorHasErrors[i] << motors[i]);
            }
        }
    }

    {
        auto radioLock = radioError.lock();

        if (radioLock->initialized)
        {
            radioInitialized();
            setError(ERR_RADIO_BOOT_FAIL, false);
        }
        else
        {
            setError(ERR_RADIO_BOOT_FAIL, true);
        }

        if (!radioLock->isValid || radioLock->hasConnectionError || radioLock->hasSoccerConnectionError)
        {
            errors |= (1 << ERR_LED_RADIO);
        }

        if (radioLock->hasConnectionError)
        {
            setError(ERR_RADIO_WIFI_FAIL, false);
        }
        else
        {
            setError(ERR_RADIO_WIFI_FAIL, true);
        }

        if (radioLock->hasSoccerConnectionError)
        {
            setError(ERR_RADIO_SOCCER_FAIL, false);
        }
        else
        {
            setError(ERR_RADIO_SOCCER_FAIL, true);
        }
    }

    {
        auto kickerLock = kickerInfo.lock();

        if (kickerLock->initialized)
        {
            kickerInitialized();
            setError(ERR_KICKER_BOOT_FAIL, false);
        }
        else
        {
            setError(ERR_KICKER_BOOT_FAIL, true);
        }

        if (kickerLock->isValid || kickerLock->kickerHasError)
        {
            errors |= (1 << ERR_LED_KICK);
        }
    }

    {
        auto imuLock = imuData.lock();

        if (imuLock->initialized)
        {
            setError(ERR_IMU_BOOT_FAIL, false);
        }
        else
        {
            setError(ERR_IMU_BOOT_FAIL, true);
        }
    }

    {
        auto batteryLock = batteryVoltage.lock();
        uint8_t voltageIn8Bits = batteryLock->rawVoltage;
        float percentage = voltageIn8Bits / 255; //Percent of the max voltage
        percentage = percentage * 0.769; //Multiplying by the range
        voltage = percentage + 1.923; // Adding the MIN_SAFE_VOLTAGE to get battVoltagePin.getVoltage
        if (voltage < 3.3)
        {
            printf("reached if statement");
        }
    }

    //ioExpander->writeMask(errors, IOExpanderErrorLEDMask);
    //    leds[0].toggle();
    static bool state = true;
    state = !state;
    leds[0].write(state);

    displayErrors();
}

void LEDModule::fpgaInitialized()
{
    leds[0] = 1;
}

void LEDModule::radioInitialized()
{
    leds[1] = 1;
}

void LEDModule::kickerInitialized()
{
    leds[2] = 1;
}

void LEDModule::fullyInitialized()
{
    leds[3] = 1;
}

void LEDModule::missedSuperLoop()
{
    for (int i = 1; i < 4; i++)
    {
        leds[i] = missedSuperLoopToggle;
    }

    missedSuperLoopToggle = !missedSuperLoopToggle;
}

void LEDModule::missedModuleRun()
{
    for (int i = 1; i < 4; i++)
    {
        if (i % 2 == 0)
        {
            leds[i] = (int)missedModuleRunToggle;
        }
        else
        {
            leds[i] = (int)!missedSuperLoopToggle;
        }
    }

    missedModuleRunToggle = !missedModuleRunToggle;
}

void LEDModule::setColor(uint32_t led0, uint32_t led1, uint32_t led2)
{
    // 0 - 31
    uint8_t brightness = 2 | 0xE0;

    std::vector<uint8_t> data = {
        0x00, 0x00, 0x00, 0x00,
        brightness, (led0 >> 0) & 0xFF, (led0 >> 8) & 0xFF, (led0 >> 16) & 0xFF,
        brightness, (led1 >> 0) & 0xFF, (led1 >> 8) & 0xFF, (led1 >> 16) & 0xFF,
        brightness, (led2 >> 0) & 0xFF, (led2 >> 8) & 0xFF, (led2 >> 16) & 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF};
    {
        auto dotStarSPILock = dotStarSPI.lock();
        dotStarSPILock->frequency(200'000);
        dotStarNCS.write(false);
        dotStarSPILock->transmitReceive(data);
        dotStarNCS.write(true);
    }
}

void LEDModule::displayErrors()
{
    // If there are no errors, then there is no point in displaying anything
    if (std::none_of(errToggles.begin(), errToggles.end(), [](bool b)
                     { return b; }))
    {
        setColor(0x000000, 0x000000, 0x000000);
    }
    else if (lightsOn && framesOnCounter >= framesOn)
    {
        // If error lights have been on long enough, switch them off
        framesOnCounter = 0;
        lightsOn = false;
        setColor(0x000000, 0x000000, 0x000000);
    }
    else if (!lightsOn && framesOffCounter >= framesOff)
    {
        // If error lights have been off long enough, switch them back on to next error color
        framesOffCounter = 0;
        lightsOn = true;

        // Make sure index loops back if it has reached end of vector
        // Since we know there is at least one error, we can safely use a while loop to find it.
        do
        {
            index = (index + 1) % errToggles.size();
        } while (!errToggles.at(index));

        Error e = ERR_LIST.at(index);
        setColor(e.led0, e.led1, e.led2);
    }
    else
    {
        // Increment frame counters for lights on/off
        lightsOn ? framesOnCounter++ : framesOffCounter++;
    }
}

void LEDModule::setError(const Error e, bool toggle)
{
    const Error *error = std::find(ERR_LIST.begin(), ERR_LIST.end(), e);
    if (error == ERR_LIST.end())
    {
        printf("[ERROR] Invalid error code\r\n");
    }
    else
    {
        errToggles.at(std::distance(ERR_LIST.begin(), error)) = toggle;
    }
}
