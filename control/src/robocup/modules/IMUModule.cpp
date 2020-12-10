#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include <cstdio>

IMUModule::IMUModule(std::shared_ptr<I2C> sharedI2C, LockedStruct<IMUData>& imuData)
    : GenericModule(kPeriod, "imu", kPriority),
      imu(sharedI2C), imuData(imuData) {
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

    imu.setTempFIFOEnabled(false);
    imu.setXGyroFIFOEnabled(false);
    imu.setYGyroFIFOEnabled(false);
    imu.setZGyroFIFOEnabled(false);
    imu.setAccelFIFOEnabled(false);
    imu.setSlave2FIFOEnabled(false);
    imu.setSlave1FIFOEnabled(false);
    imu.setSlave0FIFOEnabled(false);

    // Set rate to be 125hz
    imu.setRate(7);
    imu.setExternalFrameSync(MPU6050_EXT_SYNC_DISABLED);
    imu.setDLPFMode(MPU6050_DLPF_BW_188); // 188hz bandwidth, 2ms lag, lpf
    imu.setIntEnabled(0);
    imu.setFIFOEnabled(false);
    imu.setIntI2CMasterEnabled(false);

    // Resets analog paths to restart config
    imu.resetTemperaturePath();
    imu.resetAccelerometerPath();
    imu.resetGyroscopePath();

    imu.setWaitForExternalSensorEnabled(false);
    imu.setSlave3FIFOEnabled(false);
    imu.setMultiMasterEnabled(false);
    imu.setWakeCycleEnabled(false);

    // Set to most sensitive since we will most likely stay
    // below these rangets
    imu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    imu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

    imu.setFSyncInterruptEnabled(false);

    imu.setClockSource(1);

    imu.setInterruptMode(0);
    imu.setInterruptDrive(0);
    imu.setInterruptLatch(0); // check this if doesn't work
    imu.setIntDataReadyEnabled(true);
    imu.setInterruptLatchClear(1);
    imu.setIntDataReadyEnabled(true);


    printf("INFO: IMU initialized\r\n");
    imuData.lock()->initialized = true;
}

void IMUModule::entry(void) {
    // IMU removed
    // Occasionally the MPU6050 holds the data line low
    // causing a consistent timeout on the i2c bus
    // We tried to recover the bus by clocking out a ton
    // of just clock signals, but it did not seem to solve
    // the problem
    // A new imu is on the docket
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

    auto imuDataLock = imuData.lock();
    imuDataLock->isValid = true;
    imuDataLock->lastUpdate = HAL_GetTick();

    for (int i = 0; i < 3; i++) {
        imuDataLock->accelerations[i] = motion[i] * convertAccel;
        imuDataLock->omegas[i] = motion[i+3] * convertGyro * degToRad;
    }
}
