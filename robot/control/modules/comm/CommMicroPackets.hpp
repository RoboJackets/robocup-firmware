#pragma once

#include <cstdint>

// Mirror the the packets found in rtp
// Split up into reasonable "micro packets" such that one large packet
// doesn't need to be passed around
//
// Also allows for non-scaled form for simple interface with real code

// TODO: I really want to rename these fields in the rtp packet
//
// Make all the healthy/error type vars have the same name
//      kickHealthy -> kickerErrors
//      fpgaStatus -> fpgaError
//
// Sensor readings should be more descriptive
//      ballSenseStatus -> ballSenseTripped (or maybe hasBall)
//      kickStatus -> kickerCharged


// Bundle up commands from the rtc
struct KickerCommand {
    bool valid;

    // Enum underlying type should be the same as the values found
    // in the packets
    enum ShootMode { KICK = 0, CHIP = 1 };
    enum TriggerMode { OFF = 0, IMMEDIATE = 1, ON_BREAK_BEAM = 2, INVALID = 3 };

    ShootMode shootMode; // Should probably be a different name than the type
    TriggerMode triggerMode;

    uint8_t kickStrength;
};

struct MotorCommand {
    bool valid;

    double bodyX;
    double bodyY;
    double bodyW;
    int8_t dribbler; // Why is this int8 in the packet??
};


// Bundle up the return status to clean up everything above
struct KickerStatus {
    bool kickStatus;
    bool kickHealthy;
};

struct MotorStatus {
    bool motorErrors[4];
    int16_t encDeltas[4];
};

struct RobotStatus {
    uint8_t uid;
    uint8_t battVoltage;
    bool ballSenseStatus;
    bool fpgaStatus;
};