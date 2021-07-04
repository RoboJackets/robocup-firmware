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

    printf("INFO: IMU initialized\r\n");
    imuData.lock()->initialized = true;
}

void IMUModule::entry(void) {
    float rate = imu.gyro_z();

    auto imuDataLock = imuData.lock();
    imuDataLock->isValid = true;
    imuDataLock->lastUpdate = xTaskGetTickCount();

    imuDataLock->omegas[2] = rate;
}
