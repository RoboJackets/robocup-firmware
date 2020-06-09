#pragma once

#include <array>
#include <memory>

#include "LockedStruct.hpp"
#include "DigitalOut.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/MCP23017.hpp"

class LEDModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that LEDModule should run (Hz)
     */
    static constexpr float kFrequency = 10.0f;

    /**
     * Number of seconds elapsed (period) between LEDModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     *
     * Uses the default priority of 1.
     */
    static constexpr int kPriority = 1;

    /**
     * Constructor for LEDModule
     *
     * @param ioExpander Interfaces with MCP23017
     * @param dotStarSPI SPI bus for dotStar LEDS
     * @param batteryVoltage Packet of data containing data on battery voltage and critical status
     * @param fpgaStatus Packet of data containing whether motors or FPGA have errors
     * @param kickerInfo Packet of data containing kicker status
     * @param radioError Packet of data containing whether radio has an error
     */
    LEDModule(LockedStruct<MCP23017>& ioExpander,
              LockedStruct<SPI>& dotStarSPI,
              LockedStruct<BatteryVoltage>& batteryVoltage,
              LockedStruct<FPGAStatus>& fpgaStatus,
              LockedStruct<KickerInfo>& kickerInfo,
              LockedStruct<RadioError>& radioError);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS
     *
     * Checks for Radio, FPGA, and Kicker errors, and activates dotStar LEDs accordingly
     */
    void entry() override;

    /**
     * Set specific LED pattern for fpga initialization
     */
    void fpgaInitialized();

    /**
     * Set specific LED pattern for radio initialization
     */
    void radioInitialized();

    /**
     * Set specific LED pattern for kicker initialization
     */
    void kickerInitialized();

    /**
     * Set specific LED pattern for full system initialization
     */
    void fullyInitialized();

    /**
     * Set specific toggling pattern for missing the X ms super loop timings
     *
     * All LEDs on the mTrain are blinking together, indicating that some module is running too slowly.
     */
    void missedSuperLoop();

    /**
     * Specific toggling pattern for missing a module run X times in a row
     *
     * LEDs 1 and 3 toggle opposite of LEDs 2 and 4, indicating that due to priority and timing, some module never runs
     */
    void missedModuleRun();

private:
    /**
     * Sets the color of the two dot stars
     *
     * @param led0 0..7 red
     *             8..15 green
     *             16..23 blue
     *             24..31 don't care
     * 
     * @param led1 0..7 red
     *             8..15 green
     *             16..23 blue
     *             24..31 don't care
     */
    void setColor(uint32_t led0, uint32_t led1);

    const static uint16_t IOExpanderErrorLEDMask = 0xFF00;

    LockedStruct<MCP23017>& ioExpander;
    LockedStruct<SPI>& dotStarSPI;

    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<KickerInfo>& kickerInfo;
    LockedStruct<RadioError>& radioError;

    DigitalOut dotStarNCS;

    std::array<DigitalOut, 4> leds;
    bool missedSuperLoopToggle;
    bool missedModuleRunToggle;
};