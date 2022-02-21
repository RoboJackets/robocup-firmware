#pragma once

#include <algorithm>
#include <array>
#include <memory>

#include "GenericModule.hpp"
#include "drivers/MCP23017.hpp"


/**
 * Constants for DotStar LEDs (RGB)
 * First LED in error display format: Category / Level / Info
 */
enum CategoryColors : uint32_t {
    RADIO_ERROR = 0xFF0000,  // RED
    FPGA_ERROR = 0x00FF00,    // GREEN
    KICKER_ERROR = 0x0000FF, // BLUE
    IMU_ERROR = 0xFFFFFF   // WHITE
};

/**
 * Constants for DotStar LEDs (RGB)
 * Second LED in error display format: Category / Level / Info
 */
enum LevelColors : uint32_t {
    FATAL = 0xFF0000,  // RED
    ERR = 0x00FF00,    // GREEN
    WARN = 0x0000FF, // BLUE
    INFO = 0xFFFFFF   // WHITE
};

/**
 * Constants for DotStar LEDs (RGB)
 * Third LED in error display format: Category / Level / Info
 */
enum InfoColors : uint32_t {
    // GENERAL
    BOOT_FAIL = 0xFF0000,  // RED

    // RADIO
    RADIO_CONN_SOCCER_FAIL = 0x00FF00, // GREEN
    RADIO_CONN_WIFI_FAIL = 0x0000FF  // BLUE
};

/**
 * Struct to store LED values in ERR_LIST
 */
struct Error {
    uint32_t led0;
    uint32_t led1;
    uint32_t led2;
};

bool operator==(const Error& e1, const Error& e2);

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
     * Internally converts RGB (more readable and practical) to BGR (for dotStars)
     *
     * @param led0 Color in RGB format
     * @param led1 Color in RGB format
     * @param led2 Color in RGB format
     */
    void setColor(uint32_t led0, uint32_t led1, uint32_t led2);

    /**
     * Cycle through error color codes to display
     */
    void displayErrors();

    /**
     * Toggle error in errToggles
     */
    void setError(const Error e, bool toggle);

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
    const struct Error ERR_RADIO_BOOT_FAIL = {CategoryColors::RADIO_ERROR,
                                              LevelColors::FATAL,
                                              InfoColors::BOOT_FAIL};
    const struct Error ERR_RADIO_WIFI_FAIL = {CategoryColors::RADIO_ERROR,
                                              LevelColors::FATAL,
                                              InfoColors::RADIO_CONN_WIFI_FAIL};
    const struct Error ERR_RADIO_SOCCER_FAIL = {CategoryColors::RADIO_ERROR,
                                                LevelColors::FATAL,
                                                InfoColors::RADIO_CONN_SOCCER_FAIL};
    // FPGA
    const struct Error ERR_FPGA_BOOT_FAIL = {CategoryColors::FPGA_ERROR,
                                             LevelColors::FATAL,
                                             InfoColors::BOOT_FAIL};

    // KICKER
    const struct Error ERR_KICKER_BOOT_FAIL = {CategoryColors::KICKER_ERROR,
                                               LevelColors::FATAL,
                                               InfoColors::BOOT_FAIL};
    // IMU
    const struct Error ERR_IMU_BOOT_FAIL =  {CategoryColors::IMU_ERROR,
                                             LevelColors::FATAL,
                                             InfoColors::BOOT_FAIL};

    const std::array<Error, 6> ERR_LIST = {ERR_RADIO_BOOT_FAIL,
                                           ERR_RADIO_WIFI_FAIL,
                                           ERR_RADIO_SOCCER_FAIL,
                                           ERR_FPGA_BOOT_FAIL,
                                           ERR_KICKER_BOOT_FAIL,
                                           ERR_IMU_BOOT_FAIL};

    /**
     * Array of toggles whose values indicates where the error at the same index in ERR_LIST is active or not
     */
    std::array<bool, 6> errToggles;

    /**
     * Current index of errToggles
     */
    size_t index = 0;

    /**
     * Number of frames (1/kPeriod seconds) for which the error lights will be on, to cycle through error LEDs
     */
    int framesOn = 20;
    int framesOnCounter = 0;

    /**
     * Number of frames (1/kPeriod seconds) for which the error lights will be off, to cycle through error LEDs
     */
    int framesOff = 10;
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
