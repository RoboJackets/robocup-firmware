#include "modules/IMUModule.hpp"
#include "mtrain.hpp"

IMUModule::IMUModule(IMUData *const imuData)
    : imuData(imuData) /**, imu()**/ {
    
    // todo init imu
    imu.initialize();
    // todo setup for max rate

    imuData->isValid = false;
    imuData->lastUpdate = 0;

    for (int i = 0; i < 3; i++) {
        imuData->accelerations[i] = 0.0f;
        imuData->omegas[i] = 0.0f;
    }
}

void IMUModule::entry(void) {
    int16_t motion[6];

    imu.getMotion6(&motion[0], &motion[1], &motion[2],
                  &motion[3], &motion[4], &motion[5]);

    // todo convert to float

    imuData->isValid = true;
    imuData->lastUpdate = HAL_GetTick();

    for (int i = 0; i < 3; i++) {
        imuData->accelerations[i] = 0.0f;
        imuData->omegas[i] = 0.0f;
    }
}