#include "GenericModule.hpp"
#include "../MicroPackets.hpp" //todo setup include dir correctly

class LEDModule : public GenericModule {
public:
    // How many times per second this module should run
    const static float freq = 1.0f; // Hz
    const static uint32_t period = (uint32_t) (1000 / freq);

    // How long a single call to this module takes
    const static uint32_t runtime = 0; // ms

    LEDModule(BatteryVoltage *const batteryVoltage,
              FPGAStatus *const fpgaStatus,
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
    BatteryVoltage *const batteryVoltage;
    FPGAStatus *const fpgaStatus;
    RadioError *const radioError;
    
    // All the leds
    // state variables    
};