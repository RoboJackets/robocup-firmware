#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include <cmath>
#include "interrupt_in.h"

volatile bool dataToRead = false;
MPU6050* imu;
IMUData * imuData_m;

void int_cb() {
    dataToRead = true;
}

IMUModule::IMUModule(std::shared_ptr<I2C> sharedI2C, IMUData * imuData){ //, imu(sharedI2C) // imuData(imuData)
    imuData_m = imuData;
    
    imu = new MPU6050(sharedI2C);

    // todo init imu
    imu->initialize();
    // todo setup for max rate

    imu->setTempFIFOEnabled(false);
    imu->setXGyroFIFOEnabled(false);
    imu->setYGyroFIFOEnabled(false);
    imu->setZGyroFIFOEnabled(false);
    imu->setAccelFIFOEnabled(false);
    imu->setSlave2FIFOEnabled(false);
    imu->setSlave1FIFOEnabled(false);
    imu->setSlave0FIFOEnabled(false);

    // Set rate to be 125hz
    imu->setRate(7);
    imu->setExternalFrameSync(MPU6050_EXT_SYNC_DISABLED);
    imu->setDLPFMode(MPU6050_DLPF_BW_188); // 188hz bandwidth, 2ms lag, lpf
    imu->setIntEnabled(0);
    imu->setFIFOEnabled(false);
    imu->setIntI2CMasterEnabled(false);

    // Resets analog paths to restart config
    imu->resetTemperaturePath();
    imu->resetAccelerometerPath();
    imu->resetGyroscopePath();

    imu->setWaitForExternalSensorEnabled(false);
    imu->setSlave3FIFOEnabled(false);
    imu->setMultiMasterEnabled(false);
    imu->setWakeCycleEnabled(false);

    // Set to most sensitive since we will most likely stay
    // below these rangets
    imu->setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    imu->setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

    imu->setFSyncInterruptEnabled(false);

    imu->setClockSource(1);

    interruptin_init_ex(PIN16, &int_cb, PULL_DOWN, INTERRUPT_RISING);

    imu->setInterruptMode(0);
    imu->setInterruptDrive(0);
    imu->setInterruptLatch(0); // check this if doesn't work
    imu->setIntDataReadyEnabled(true);
    imu->setInterruptLatchClear(1);
    imu->setIntDataReadyEnabled(true);


    printf("INFO: IMU initialized\r\n");

    imuData_m->isValid = false;
    imuData_m->lastUpdate = 0;

    for (int i = 0; i < 3; i++) {
        imuData_m->accelerations[i] = 0.0f;
        imuData_m->omegas[i] = 0.0f;
    }
}

void IMUModule::entry(void) {
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

    if (dataToRead) {

        imuData_m->isValid = true;
        imuData_m->lastUpdate = HAL_GetTick();
        imuData_m->omegas[2] = imu->getRotationZ() * convertGyro * degToRad;
        dataToRead = false;
    }
    return;

    imu->getMotion6(&motion[0], &motion[1], &motion[2],
                   &motion[3], &motion[4], &motion[5]);

    imuData_m->isValid = true;
    imuData_m->lastUpdate = HAL_GetTick();

    for (int i = 0; i < 3; i++) {
        imuData_m->accelerations[i] = motion[i] * convertAccel;
        imuData_m->omegas[i] = motion[i+3] * convertGyro * degToRad;
    }
}