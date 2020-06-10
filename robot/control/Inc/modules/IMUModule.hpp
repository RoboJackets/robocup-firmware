#pragma once

#include "I2C.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MPU6050.h"
#include "LockedStruct.hpp"
#include <memory>

/**
 * Module interfacing with IMU and handling IMU status
 */
class IMUModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that BatteryModule should run (Hz)
     */
    static constexpr float kFrequency = 200.0f;

    /**
     * Number of seconds elapsed (period) between FPGAModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for IMUModule
     * @param sharedI2C Pointer to I2C object which reads/writes on I2C bus
     * @param imuData Packet of IMU data sent between IMUModule and MotionControlModule
     */
    IMUModule(std::shared_ptr<I2C> sharedI2C, LockedStruct<IMUData>& imuData);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS
     *
     * @note As of May 2020, IMUModule is not integrated on the robots due to issues. The code immediately returns.
     */
    void entry() override;

private:
    MPU6050 imu;
    LockedStruct<IMUData>& imuData;
};