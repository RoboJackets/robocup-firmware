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
    /**
     * Number of times per second (frequency) that LEDModule should run (Hz)
     */
    static constexpr float kFrequency = 200.0f;

    /**
     * Number of seconds elapsed (period) between LEDModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for MotionControlModule
     * @param batteryVoltage Packet of data containing data on battery voltage and critical status
     * @param imuData Packet of data containing linear acceleration and angular velocity along/about X,Y, and Z axes
     * @param motionCommand Packet of data containing dribbler rotation, x and y linear velocity, z angular velocity
     * @param motorFeedback Packet of data containing encoder counts and currents to each wheel motor
     * @param motorCommand Packet of data containing wheel motor duty cycles and dribbler rotation speed
     */
    MotionControlModule(LockedStruct<BatteryVoltage>& batteryVoltage,
                        LockedStruct<IMUData>& imuData,
                        LockedStruct<MotionCommand>& motionCommand,
                        LockedStruct<MotorFeedback>& motorFeedback,
                        LockedStruct<MotorCommand>& motorCommand);

    /**
     * Code to run when called by RTOS
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
