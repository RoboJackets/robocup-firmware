#pragma once

#include "modules/GenericModule.hpp"
#include "MicroPackets.hpp"

#include "motion-control/DribblerController.hpp"
#include "motion-control/RobotController.hpp"
#include "motion-control/RobotEstimator.hpp"

#include <Eigen/Dense>
#include "LockedStruct.hpp"

class MotionControlModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 200.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 3;

    MotionControlModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                        LockedStruct<IMUData>& imuData,
                        LockedStruct<MotionCommand>& motionCommand,
                        LockedStruct<MotorFeedback>& motorFeedback,
                        LockedStruct<MotorCommand>& motorCommand);

    virtual void entry(void);

private:
    /**
     * Checks the update time versus the current time and timout given below
     */
    bool isRecentUpdate(uint32_t lastUpdateTime);

    LockedStruct<BatteryVoltage>& batteryVoltage;
    LockedStruct<IMUData>& imuData;
    LockedStruct<MotionCommand>& motionCommand;
    LockedStruct<MotorFeedback>& motorFeedback;
    LockedStruct<MotorCommand>& motorCommand;

    DribblerController dribblerController;
    RobotController robotController;
    RobotEstimator robotEstimator;

    Eigen::Matrix<double, 4, 1> prevCommand;

    /**
     * Max amount of time that can elapse from the latest
     * command from the radio
     * 
     * This is a safety feature to prevent the motors from moving
     * if the motion control dies
     */
    static constexpr uint32_t COMMAND_TIMEOUT = 250; // ms
};
