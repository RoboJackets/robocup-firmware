#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include <cmath>
#include "interrupt_in.h"
#include "PinDefs.hpp"

IMUModule::IMUModule(std::shared_ptr<SPI> sharedSPI, IMUData* imuData)
        : imu_data(imuData), imu(sharedSPI, p18) {
    // Initialize the IMU
    imu.initialize();

    for (int i = 0; i < 3; i++) {
        imu_data->accelerations[i] = 0.0f;
        imu_data->omegas[i] = 0.0f;
    }
}

void IMUModule::entry(void) {
    imu_data->omegas[2] = imu.gyro_z();
    imu_data->isValid = true;
    imu_data->lastUpdate = HAL_GetTick();
}