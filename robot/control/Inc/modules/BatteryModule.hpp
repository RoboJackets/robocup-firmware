#include "GenericModule.hpp"
#include "MicroPackets.hpp" //todo setup include dir correctly

#include "drivers/Battery.hpp"

class BatteryModule : public GenericModule {
public:
    // How many times per second this module should run
    const static float freq = 1.0f; // Hz
    const static uint32_t period = (uint32_t) (1000 / freq);

    // How long a single call to this module takes
    const static uint32_t runtime = 0; // ms

    BatteryModule(BatteryVoltage *const batteryVoltage);

    virtual void entry(void);

private:
    BatteryVoltage *const batteryVoltage;
    
    Battery battery;
};