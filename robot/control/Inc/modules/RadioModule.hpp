#pragma once

#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "radio/RadioLink.hpp"

class RadioModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 30.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 5; // ms

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

    RadioLink link;
};