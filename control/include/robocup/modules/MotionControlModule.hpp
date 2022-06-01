#pragma once

#include "modules/GenericModule.hpp"
#include "MicroPackets.hpp"

#include "motion-control/DribblerController.hpp"
#include "motion-control/RobotController.hpp"
#include "motion-control/RobotEstimator.hpp"

#include <Eigen/Dense>
#include "LockedStruct.hpp"

/**
 * Module handling robot state estimation and motion control for motors
 */
class MotionControlModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that MotionControlModule should run (Hz)
     */
    static constexpr float kFrequency = 100.0f;

    /**
     * Number of seconds elapsed (period) between MotionControlModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for MotionControlModule
     * @param batteryVoltage Shared memory location containing data on battery voltage and critical status
     * @param imuData Shared memory location containing linear acceleration and angular velocity along/about X,Y, and Z axes
     * @param motionCommand Shared memory location containing dribbler rotation, x and y linear velocity, z angular velocity
     * @param motorFeedback Shared memory location containing encoder counts and currents to each wheel motor
     * @param motorCommand Shared memory location containing wheel motor duty cycles and dribbler rotation speed
     */
    MotionControlModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                        LockedStruct<IMUData>& imuData,
                        LockedStruct<MotionCommand>& motionCommand,
                        LockedStruct<MotorFeedback>& motorFeedback,
                        LockedStruct<MotorCommand>& motorCommand,
                        LockedStruct<DebugInfo>& debugInfo);

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * Uses IMU Data, motion data, motor data, and battery voltage data to update robot state estimation
     * and motor controllers.
     */
    void entry() override;

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
    LockedStruct<DebugInfo>& debugInfo;

    DribblerController dribblerController;
    RobotController robotController;
    RobotEstimator robotEstimator;

    Eigen::Matrix<float, 4, 1> prevCommand;

    /**
     * Max amount of time that can elapse from the latest
     * command from the radio (milliseconds)
     *
     * This is a safety feature to prevent the motors from moving
     * if the motion control dies
     */
    static constexpr uint32_t COMMAND_TIMEOUT = 250;
};
