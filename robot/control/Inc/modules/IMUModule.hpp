#pragma once

#include "I2C.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MPU6050.h"
#include "LockedStruct.hpp"
#include <memory>

class IMUModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float kFrequency = 200.0f; // Hz
    static constexpr std::chrono::milliseconds kPeriod{static_cast<int>(1000 / kFrequency)};
    static constexpr int kPriority = 3;

    IMUModule(std::shared_ptr<I2C> sharedI2C, LockedStruct<IMUData>& imuData);

    virtual void entry(void);

private:
    MPU6050 imu;
    LockedStruct<IMUData>& imuData;
};