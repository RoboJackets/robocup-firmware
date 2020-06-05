#pragma once

#include "LockedStruct.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "radio/RadioLink.hpp"
#include "DigitalOut.hpp"

class RadioModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that KickerModule should run (Hz)
     */
    static constexpr float kFrequency = 50.0f;

    /**
     * Number of seconds elapsed (period) between KickerModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for RadioModule
     * @param batteryVoltage Packet of data containing data on battery voltage and critical status
     * @param fpgaStatus Packet of data containing whether motors or FPGA have errors
     * @param kickerInfo Packet of data containing kicker status
     * @param robotID Packet of data containing ID selected for Robot on rotary dial
     * @param kickerCommand Packet of data containing kicker shoot mode, trigger mode, and kick strength
     * @param motionCommand Packet of data containing dribbler rotation, x and y linear velocity, z angular velocity
     * @param radioError Packet of data containing whether radio has an error
     */
    RadioModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                LockedStruct<FPGAStatus>& fpgaStatus,
                LockedStruct<KickerInfo>& kickerInfo,
                LockedStruct<RobotID>& robotID,
                LockedStruct<KickerCommand>& kickerCommand,
                LockedStruct<MotionCommand>& motionCommand,
                LockedStruct<RadioError>& radioError);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS
     *
     * Sends `batteryVoltage`, `fpgaStatus`, `kickerInfo`, `robotID` packets to radio
     * Receives `kickerCommand`, `motionCommand` packets from radio
     */
    void entry() override;

private:
    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<FPGAStatus>& fpgaStatus;
    LockedStruct<KickerInfo>& kickerInfo;
    LockedStruct<RobotID>& robotID;
    
    LockedStruct<KickerCommand>& kickerCommand;
    LockedStruct<MotionCommand>& motionCommand;
    LockedStruct<RadioError>& radioError;

    /**
     * Object which sends and receives radio packets
     */
    RadioLink link;
    
    DigitalOut secondRadioCS;
};