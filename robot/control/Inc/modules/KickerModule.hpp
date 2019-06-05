#include "GenericModule.hpp"
#include "../MicroPackets.hpp" //todo setup include dir correctly
#include "drivers/KickerBoard.hpp"

class KickerModule : public GenericModule {
public:
    // How many times per second this module should run
    const static float freq = 100.0f; // Hz
    const static uint32_t period = (uint32_t) (1000 / freq);

    // How long a single call to this module takes
    const static uint32_t runtime = 1; // ms

    KickerModule(KickerCommand *const kickerCommand,
                 KickerInfo *const kickerInfo);

    virtual void entry(void);

private:
    KickerCommand *const kickerCommand;
    KickerInfo *const kickerInfo;
    
    KickerBoard kicker;
};