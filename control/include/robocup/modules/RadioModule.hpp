#pragma once

#include "DigitalOut.hpp"
#include "GenericModule.hpp"
#include "LockedStruct.hpp"
#include "MicroPackets.hpp"
#include "radio/RadioLink.hpp"

/**
 * Module interfacing with Radio and handling Radio status
 */
class RadioModule : public GenericModule {
public:
/**
     * Number of times per second (frequency) that RadioModule should run (Hz)
     */
    static constexpr float kFrequency = 90.0f;

    /**
     * Number of seconds elapsed (period) between RadioModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 5;

    /**
     * Constructor for RadioModule
     * @param batteryVoltage Shared memory location containing data on battery voltage and critical status
     * @param fpgaStatus Shared memory location containing whether motors or FPGA have errors
     * @param kickerInfo Shared memory location containing kicker status
     * @param robotID Shared memory location containing ID selected for Robot on rotary dial
     * @param kickerCommand Shared memory location containing kicker shoot mode, trigger mode, and kick strength
     * @param motionCommand Shared memory location containing dribbler rotation, x and y linear velocity, z angular velocity
     * @param radioError Shared memory location containing whether radio has an error
     */
    RadioModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                LockedStruct<FPGAStatus>& fpgaStatus,
                LockedStruct<KickerInfo>& kickerInfo,
                LockedStruct<RobotID>& robotID,
                LockedStruct<KickerCommand>& kickerCommand,
                LockedStruct<MotionCommand>& motionCommand,
                LockedStruct<RadioError>& radioError,
                LockedStruct<DebugInfo>& debugInfo,
                LockedStruct<LEDCommand>& ledCommand);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
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
    LockedStruct<DebugInfo>& debugInfo;
    LockedStruct<LEDCommand>& ledCommand;

    /**
     * General radio driver interface acting as a middle man to send and receive radio packets
     */
    RadioLink link;
    DigitalOut secondRadioCS;

    /**
     * Send when this mode is even. Receive when it is odd.
     */
    uint8_t current_mode;

    /**
     * Choose an odd divisor to receive more than send.
     */
    static constexpr uint8_t mode_divisor = 2;

    // not necessary,
    // but let's impose that the divisor evenly divides the frequency
    static_assert((int)kFrequency % mode_divisor == 0,
                  "The mode divisor should evenly divide the radio module's frequency");

    void fakeEntry();
    void realEntry();

    void send();
    void receive();
};
