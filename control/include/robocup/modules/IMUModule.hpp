#pragma once

#include <cstdio>
#include <memory>

#include "GenericModule.hpp"
#include "drivers/ICM42605.hpp"

/**
 * Module interfacing with IMU and handling IMU status
 */
class IMUModule : public GenericModule {
public:
    /**
     * Number of times per second (frequency) that IMUModule should run (Hz)
     */
    static constexpr float kFrequency = 200.0f;

    /**
     * Number of seconds elapsed (period) between IMUModule runs (milliseconds)
     */
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};

    /**
     * Priority used by RTOS
     */
    static constexpr int kPriority = 3;

    /**
     * Constructor for IMUModule
     * @param sharedSPI Pointer to SPI object which reads/writes on SPI bus
     * @param imuData Shared memory location containing linear acceleration and angular velocity along/about X,Y, and Z axes
     */
    IMUModule(LockedStruct<SPI>& sharedSPI, LockedStruct<IMUData>& imuData);

    /**
     * Code which initializes module
     */
    void start() override;

    /**
     * Code to run when called by RTOS once per system tick (`kperiod`)
     *
     * @note As of May 2020, IMUModule is not integrated on the robots due to issues. The code immediately returns.
     */
    void entry() override;

private:
    ICM42605 imu;
    LockedStruct<IMUData>& imuData;
};
