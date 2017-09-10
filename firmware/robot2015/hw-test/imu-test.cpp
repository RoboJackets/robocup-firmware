#include "Mbed.hpp"
#include "Rtos.hpp"

#include <algorithm>
#include <vector>

#include <HelperFuncs.hpp>

#include "RobotDevices.hpp"

#include "mpu-6050.hpp"
#include "pins-ctrl-2015.hpp"

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

    /*
    i2c.frequency(100000);

    while (true) {
        int addr = 0x68;
        const char GYRO_XOUT_H = 0x43;
        i2c.write(addr<<1, &GYRO_XOUT_H, 1);

        char data[2];
        i2c.read(addr<<1, data, 2);

        printf("%d, %d\r\n", data[0], data[1]);
        wait(0.25);
    }
    */

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
    while (true) {
        printf("Test passed? %d\r\n", imu.testConnection());
        //printf("Test passed? %d\r\n", imu.getGyroRawX());
        imu.getGyro(return_gyro_vals);
        printf("%f\r\n", return_gyro_vals[2]);
        wait(0.25);
    }

}
