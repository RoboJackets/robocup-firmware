#pragma once

#include "LockedStruct.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "radio/RadioLink.hpp"
#include "DigitalOut.hpp"

class RadioModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 50.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 3;

    RadioModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                LockedStruct<FPGAStatus>& fpgaStatus,
                LockedStruct<KickerInfo>& kickerInfo,
                LockedStruct<RobotID>& robotID,
                LockedStruct<KickerCommand>& kickerCommand,
                LockedStruct<MotionCommand>& motionCommand,
                LockedStruct<RadioError>& radioError);

    virtual void entry(void);

private:
    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<KickerInfo>& kickerInfo;
    LockedStruct<RobotID>& robotID;
    
    LockedStruct<KickerCommand>& kickerCommand;
    LockedStruct<MotionCommand>& motionCommand;
    LockedStruct<RadioError>& radioError;

    RadioLink link;
    DigitalOut secondRadioCS;
};