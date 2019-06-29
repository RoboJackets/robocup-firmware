#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include <cmath>

IMUModule::IMUModule(std::shared_ptr<I2C> sharedI2C, IMUData *const imuData)
    : imuData(imuData), imu(sharedI2C) {
    
    // todo init imu
    imu.initialize();
    // todo setup for max rate

    // Setup lpf for a queue of 1
    // Causes gyro output rate to be 1k
    // Set sample rate to be 1k
    imu.setRate(0);
    imu.setExternalFrameSync(MPU6050_EXT_SYNC_DISABLED);
    imu.setDLPFMode(MPU6050_DLPF_BW_188); // 188hz bandwidth, 2ms lag, lpf
    imu.setIntEnabled(0);

    // Resets analog paths to restart config
    imu.resetTemperaturePath();
    imu.resetAccelerometerPath();
    imu.resetGyroscopePath();

    imu.setFIFOEnabled(false);
    imu.setMultiMasterEnabled(false);
    imu.setWakeCycleEnabled(false);

    // Set to most sensitive since we will most likely stay
    // below these rangets
    imu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    imu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

    printf("INFO: IMU initialized\r\n");

    imuData->isValid = false;
    imuData->lastUpdate = 0;

    for (int i = 0; i < 3; i++) {
        imuData->accelerations[i] = 0.0f;
        imuData->omegas[i] = 0.0f;
    }
}

void IMUModule::entry(void) {
    return;
    // Accel lsb -> g conversions
    // +- 2g = 16384 lsb/g
    // +- 4g = 8192 lsb/g
    // +- 8g = 4096 lsb/g
    // +- 16g = 2048 lsb/g
    const float convertAccel = 1.0f / 16384.0f;

    // Gyro lsb -> deg/s conversions
    // +- 250 = 131 lsb/deg/s
    // +- 500 = 65.5 lsb/deg/s
    // +- 1000 = 32.8 lsb/deg/s
    // +- 2000 = 16.4 lsb/deg/s
    const float convertGyro = 1.0f / 32.8f;
    const float degToRad = M_PI / 180.0f;


    int16_t motion[6];

    imu.getMotion6(&motion[0], &motion[1], &motion[2],
                  &motion[3], &motion[4], &motion[5]);

    imuData->isValid = true;
    imuData->lastUpdate = HAL_GetTick();

    for (int i = 0; i < 3; i++) {
        imuData->accelerations[i] = motion[i] * convertAccel;
        imuData->omegas[i] = motion[i+3] * convertGyro * degToRad;
    }
}