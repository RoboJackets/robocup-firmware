#include "Mbed.hpp"
#include "Rtos.hpp"

#include <algorithm>
#include <vector>

#include <HelperFuncs.hpp>

#include "RobotDevices.hpp"

#include "mpu-6050.hpp"
#include "pins-control.hpp"

DigitalOut good(LED1, 0);
DigitalOut bad1(LED2, 0);
DigitalOut bad2(LED3, 0);
DigitalOut pwr(LED4, 1);
Serial pc(RJ_SERIAL_RXTX);
I2C i2c(p28, p27);

bool testPass = false;
bool batchedResult = false;
std::vector<unsigned int> freq1;
std::vector<unsigned int> freq2;

int main() {
    pc.baud(57600);


    MPU6050 imu(RJ_I2C_SDA, RJ_I2C_SCL);

    //imu.selfTest();
    //imu.reset();

    //imu.calibrate();

    /*
    imu.setBW(MPU6050_BW_256);
    imu.setGyroRange(MPU6050_GYRO_RANGE_250);
    imu.setAcceleroRange(MPU6050_ACCELERO_RANGE_2G);
    imu.setSleepMode(false);
    */


    float return_gyro_vals[3];
    float pos_rad = 0.0;
    const float delta = 0.05;
    while (true) {
        //printf("Test passed? %d\r\n", imu.testConnection());
        //printf("Test passed? %d\r\n", imu.getGyroRawX());
        imu.getGyro(return_gyro_vals);
        pos_rad += return_gyro_vals[2] * delta;
        printf("%f\r\n", pos_rad);

        wait(delta);
    }

}
