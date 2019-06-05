#include "GenericModule.hpp"
#include "../MicroPackets.hpp" //todo setup include dir correctly

class RadioModule : public GenericModule {
public:
    // How many times per second this module should run
    const static float freq = 50.0f; // Hz
    const static uint32_t period = (uint32_t) (1000 / freq);

    // How long a single call to this module takes
    const static uint32_t runtime = 5; // ms

    RadioModule(BatteryVoltage *const batteryVoltage,
                FPGAStatus *const fpgaStatus,
                KickerInfo *const kickerInfo,
                RobotID *const robotID,
                KickerCommand *const kickerCommand,
                MotionCommand *const motionCommand,
                RadioError *const radioError);

    virtual void entry(void);

private:
    BatteryVoltage *const batteryVoltage;
    FPGAStatus *const fpgaStatus;
    KickerInfo *const kickerInfo;
    RobotID *const robotID;
    
    KickerCommand *const kickerCommand;
    MotionCommand *const motionCommand;
    RadioError *const radioError;

    // Radio driver
};