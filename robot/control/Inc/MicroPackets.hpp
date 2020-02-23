#pragma once

#include "cstdint"

// Micropackets are shared memory locations for values
// where only the latest matters. There is no guarantee that
// data is not lost between reads.
//
// The "producer" of the data must initialize the micropacket
// The "consumer" of the data must check if it's valid and
// the data isn't too old (if applicable)

// Robot body velocities and dribbler commands
// Radio -> Motion Control
struct MotionCommand {
    bool isValid = false;
    uint32_t lastUpdate;

    float bodyXVel; // m/s
    float bodyYVel; // m/s
    float bodyWVel; // rad/s

    // Limited from 0-128
    uint8_t dribbler; // lsb
};

// Duty cycle commands for individual motors
// MotionControl -> FPGA
struct MotorCommand {
    bool isValid = false;
    uint32_t lastUpdate;

    float wheels[4]; // % max duty cycle (-1 to 1)
    uint16_t dribbler; // lsb (0 to 128)
};

// Current and enc data
// FPGA -> MotionControl
struct MotorFeedback {
    bool isValid = false;
    uint32_t lastUpdate;

    float encoders[4]; // rad / s
    float currents[4]; // amp
};

// Accelerations and angular velocities
// IMU -> MotionControl
struct IMUData {
    bool isValid = false;
    bool initialized = false;
    uint32_t lastUpdate;

    float accelerations[3]; // X Y Z (m/s2)
    float omegas[3]; // X Y Z (rad/s)
};

// Battery voltage and whether it's at a critical level
// Battery -> MotionControl
// Battery -> LED
// Battery -> Radio
struct BatteryVoltage {
    bool isValid = false;
    uint32_t lastUpdate;

    uint8_t rawVoltage; // V
    bool isCritical;
};

// Whether the FPGA and motors are working or not
// FPGA -> LED
// FPGA -> Radio
struct FPGAStatus {
    bool isValid = false;
    bool initialized = false;
    uint32_t lastUpdate;

    bool motorHasErrors[5];
    bool FPGAHasError;
};

// Whether the radio is working correctly
// Radio -> LED
struct RadioError {
    bool isValid = false;
    bool initialized = false;
    uint32_t lastUpdate;

    bool hasError;
};

// What ID we selected on the dial
// Rotary Dial -> Radio
struct RobotID {
    bool isValid = false;
    uint32_t lastUpdate;

    uint8_t robotID;
};

// Whether to kick, chip, kick on breakbeam etc
// Radio -> Kicker
struct KickerCommand {
    bool isValid = false;
    uint32_t lastUpdate;

    enum ShootMode { KICK = 0, CHIP = 1 };
    enum TriggerMode { OFF = 0, IMMEDIATE = 1, ON_BREAK_BEAM = 2, INVALID = 3 };

    ShootMode shootMode;
    TriggerMode triggerMode;

    uint8_t kickStrength;
};

// Status of kicker and breakbeam status
// Kicker -> Radio
struct KickerInfo {
    bool isValid = false;
    bool initialized = false;
    uint32_t lastUpdate;

    bool kickerHasError;
    bool kickerCharged;
    bool ballSenseTriggered;
};

// 18 ints for debug info to pass back to soccer
// Any -> Radio
//
// Use `extern DebugInfo debugInfo;` at the top of any cpp
// and then set specific values using `debugInfo.val[i] = X`
// It is up to the user to make sure the indicies don't conflict
struct DebugInfo {
    int16_t val[18];
};

