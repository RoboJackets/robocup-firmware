#pragma once

#include <algorithm>
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
 * Constants for DotStar LEDs (BGR)
 * First LED in error display format: Category / Level / Info
 */
enum errorColors_0 : uint32_t {
    RADIO_ERROR = 0x0080FF,  // ORANGE
    FPGA_ERROR = 0x00FFFF,    // YELLOW
    KICKER_ERROR = 0x00FF00, // GREEN
    IMU_ERROR = 0xFF0000   // BLUE
};

/**
 * Constants for DotStar LEDs (BGR)
 * Second LED in error display format: Category / Level / Info
 */
enum errorColors_1 : uint32_t {
    FATAL = 0x0000FF, // RED
    ERR = 0x0080FF, // ORANGE
    WARN = 0xFFFF00,  // YELLOW
    INFO = 0xFF0000  // BLUE
};

/**
 * Constants for DotStar LEDs (BGR)
 * Third LED in error display format: Category / Level / Info
 */
enum errorColors_2 : uint32_t {
    // GENERAL
    BOOT_FAIL = 0x0000FF,  // RED

    // RADIO
    RADIO_CONN_WIFI_FAIL = 0x0080FF,  // ORANGE
    RADIO_CONN_SOCCER_FAIL = 0x00FFFF // YELLOW
};

/**
 * Struct to store LED values in colorQueue
 */
struct Error {
    uint32_t led0;
    uint32_t led1;
    uint32_t led2;
};

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
              LockedStruct<RadioError>& radioError,
              LockedStruct<IMUData>& imuData);

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
     */
    void fpgaInitialized();

    /**
     * Toggles LEDs to signal radio initialization
     * - mTrain LED2 turned on
     */
    void radioInitialized();

    /**
     * Toggles LEDs to signal fpga initialization
     * - mTrain LED3 turned on
     */
    void kickerInitialized();

    /**
     * Toggles LEDs to signal full system initialization
     * - mTrain LED4 turned on
     */
    void fullyInitialized();

    /**
     * Set specific toggling pattern for missing the X ms super loop timings
     *
     * mTrain LEDs 2,3, and 4 are blinking together, indicating that some module is running too slowly.
     */
    void missedSuperLoop();

    /**
     * Set specific toggling pattern for missing a module run X times in a row
     *
     * mTrain LED 3 toggles opposite of LEDs 2 and 4, indicating that due to priority and timing, some module never runs
     */
    void missedModuleRun();

private:
    /**
     * Sets the color of the three dot stars
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
     *
     * @param led2 0..7 red
     *             8..15 green
     *             16..23 blue
     *             24..31 don't care
     */
    void setColor(uint32_t led0, uint32_t led1, uint32_t led2);

    /**
     * Cycle through error color codes to display
     */
    void displayErrors();

    /**
     * Add error to colorQueue
     */
    void addError(Error newError);

    /**
     * Remove error from colorQueue
     */
    void removeError(Error error);

    const static uint16_t IOExpanderErrorLEDMask = 0xFF00;

    LockedStruct<MCP23017>& ioExpander;
    LockedStruct<SPI>& dotStarSPI;

    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<KickerInfo>& kickerInfo;
    LockedStruct<RadioError>& radioError;
    LockedStruct<IMUData>& imuData;

    DigitalOut dotStarNCS;

    // RADIO
    const struct Error ERR_RADIO_BOOT_FAIL = {errorColors_0::RADIO_ERROR,
                                              errorColors_1::FATAL,
                                              errorColors_2::BOOT_FAIL};
    const struct Error ERR_RADIO_WIFI_FAIL = {errorColors_0::RADIO_ERROR,
                                              errorColors_1::FATAL,
                                              errorColors_2::RADIO_CONN_WIFI_FAIL};
    const struct Error ERR_RADIO_SOCCER_FAIL = {errorColors_0::RADIO_ERROR,
                                                errorColors_1::FATAL,
                                                errorColors_2::RADIO_CONN_SOCCER_FAIL};
    // FPGA
    const struct Error ERR_FPGA_BOOT_FAIL = {errorColors_0::FPGA_ERROR,
                                             errorColors_1::FATAL,
                                             errorColors_2::BOOT_FAIL};

    // KICKER
    const struct Error ERR_KICKER_BOOT_FAIL = {errorColors_0::KICKER_ERROR,
                                               errorColors_1::FATAL,
                                               errorColors_2::BOOT_FAIL};
    // IMU
    const struct Error ERR_IMU_BOOT_FAIL =  {errorColors_0::IMU_ERROR,
                                             errorColors_1::FATAL,
                                             errorColors_2::BOOT_FAIL};


    /**
     * Vector of colors for dotStars to display sequentially based on current errors
     */
    std::vector<Error> colorQueue;

    /**
     * Current index of colorQueue
     */
    size_t index = 0;

    /**
     * Number of frames (1/kPeriod seconds) for which the error lights will be on, to cycle through error LEDs
     */
    int framesOn = 2;
    int framesOnCounter = 0;

    /**
     * Number of frames (1/kPeriod seconds) for which the error lights will be off, to cycle through error LEDs
     */
    int framesOff = 3;
    int framesOffCounter = 0;

    /**
     * Toggles whether dotStar LEDs are on or off for blinking between errors
     */
    bool lightsOn = true;

    /**
     * Array of mTrain LEDs as `DigitalOut`s
     */
    std::array<DigitalOut, 4> leds;
    bool missedSuperLoopToggle;
    bool missedModuleRunToggle;
};