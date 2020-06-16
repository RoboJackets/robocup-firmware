#pragma once

#include <array>
#include <memory>
#include <vector>

#include "LockedStruct.hpp"
#include "DigitalOut.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/MCP23017.hpp"

/**
 * Module interfacing with debugging LEDS based on the statuses of other electronics
 */
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
     * @param ioExpander shared_ptr with mutex locks for MCP23017 driver
     * @param dotStarSPI SPI bus for dotStar LEDS
     * @param batteryVoltage Shared memory location containing data on battery voltage and critical status
     * @param fpgaStatus Shared memory location containing whether motors or FPGA have errors
     * @param kickerInfo Shared memory location containing kicker status
     * @param radioError Shared memory location containing whether radio has an error
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
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Checks for Radio, FPGA, and Kicker errors, and activates dotStar LEDs accordingly
     * mTrain LED 1 toggled as watchdog/running signal
     */
    void entry() override;

    /**
     * Toggles LEDs to signal fpga initialization
     * - mTrain LED1 turned on
     * - dotstars set to [yellow, white]
     */
    void fpgaInitialized();

    /**
     * Toggles LEDs to signal radio initialization
     * - mTrain LED2 turned on
     * - dotstars set to [pink, white]
     */
    void radioInitialized();

    /**
     * Toggles LEDs to signal fpga initialization
     * - mTrain LED3 turned on
     * - dotstars set to [blue, white]
     */
    void kickerInitialized();

    /**
     * Toggles LEDs to signal full system initialization
     * - mTrain LED4 turned on
     * - dotstars set to [green, green]
     */
    void fullyInitialized();

    /**
     * Set specific toggling pattern for missing the X ms super loop timings
     *
     * mTrain LEDs 2,3, and 4 are blinking together, indicating that some module is running too slowly.
     */
    void missedSuperLoop();

    /**
     * Specific toggling pattern for missing a module run X times in a row
     *
     * mTrain LED 3 toggles opposite of LEDs 2 and 4, indicating that due to priority and timing, some module never runs
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

    /**
     * Cycle through error color codes to display
     */
    void displayErrors();

    const static uint16_t IOExpanderErrorLEDMask = 0xFF00;

    LockedStruct<MCP23017>& ioExpander;
    LockedStruct<SPI>& dotStarSPI;

    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<KickerInfo>& kickerInfo;
    LockedStruct<RadioError>& radioError;

    DigitalOut dotStarNCS;

    /**
     * Vector of colors for dotStars to display sequentially based on current errors
     */
    std::vector<std::array<std::uint32_t, 2>> colorQueue;

    /**
     * Current index of displayed color
     */
    int index;

    /**
     * Array of mTrain LEDs as `DigitalOut`s
     */
    std::array<DigitalOut, 4> leds;
    bool missedSuperLoopToggle;
    bool missedModuleRunToggle;
};