#pragma once

#include "SPI.hpp"
#include "GenericModule.hpp"
#include "MicroPackets.hpp"
#include "drivers/ICM42605.hpp"
#include <memory>

class IMUModule : public GenericModule {
public:
    // How many times per second this module should run
    static constexpr float freq = 200.0f; // Hz
    static constexpr uint32_t period = static_cast<uint32_t>(1000000L / freq);

    // How long a single call to this module takes
    static constexpr uint32_t runtime = 333; // us

    IMUModule(std::shared_ptr<SPI> sharedSPI, IMUData* imuData);

    virtual void entry(void);

private:
    IMUData *const imu_data;

    ICM42605 imu;
};