#pragma once

#include <array>
#include <memory>

#include "DigitalOut.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/MCP23017.hpp"

class LEDModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 1.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 0; // ms

    LEDModule(std::shared_ptr<MCP23017> ioExpander,
              BatteryVoltage *const batteryVoltage,
              FPGAStatus *const fpgaStatus,
              KickerInfo *const kickerInfo,
              RadioError *const radioError);

    virtual void entry(void);

    // Specific LED pattern for fpga initialization
    void fpgaInitialized();

    // Specific LED pattern for radio initialization
    void radioInitialized();

    // Specific LED pattern for kicker initialization
    void kickerInitialized();

    // Specific LED pattern for fully initialized system
    void fullyInitialized();

    // Specific toggling pattern for missing the X ms
    // super loop timings
    //
    // All LED's on mtrain are blinking together
    //
    // AKA, some module is too slow
    void missedSuperLoop();

    // Specific toggling pattern for missing a module run
    // X times in a row
    //
    // LED 1&3 toggle opposite of LED 2&4
    //
    // AKA, due to priority and timing, some module never runs
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

    BatteryVoltage *const batteryVoltage;
    FPGAStatus *const fpgaStatus;
    KickerInfo *const kickerInfo;
    RadioError *const radioError;

    SPI dot_star_spi;

    std::shared_ptr<MCP23017> ioExpander;

    std::array<DigitalOut, 4> leds;
    bool missedSuperLoopToggle;
    bool missedModuleRunToggle;
};