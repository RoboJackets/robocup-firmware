#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include <cstdio>

IMUModule::IMUModule(LockedStruct<SPI>& sharedSPI, LockedStruct<IMUData>& imuData)
    : GenericModule(kPeriod, "imu", kPriority),
      imu(sharedSPI, p18), imuData(imuData) {
    auto imuDataLock = imuData.unsafe_value();
    imuDataLock->isValid = false;
    imuDataLock->lastUpdate = 0;

    for (int i = 0; i < 3; i++) {
        imuDataLock->accelerations[i] = 0.0f;
        imuDataLock->omegas[i] = 0.0f;
    }
}

void IMUModule::start() {
    imu.initialize();

    imuData.lock()->initialized = true;
    printf("[INFO] IMU module initialized\r\n");
}

void IMUModule::entry(void) {
    //printf("[INFO] IMU entry\r\n");
    float rate = imu.gyro_z();

    auto imuDataLock = imuData.lock();
    imuDataLock->isValid = true;
    imuDataLock->lastUpdate = xTaskGetTickCount();

    imuDataLock->omegas[2] = rate;
}
