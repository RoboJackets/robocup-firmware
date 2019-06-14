#pragma once

#include "I2C.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp" 
#include "drivers/MPU6050.h"
#include <memory>

class IMUModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 100.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000 / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 0; // ms

    IMUModule(std::shared_ptr<I2C> sharedI2C, IMUData *const imuData);

    virtual void entry(void);

private:
    IMUData *const imuData;

    MPU6050 imu;
};