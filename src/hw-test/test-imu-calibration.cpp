#include "Mbed.hpp"
#include "Rtos.hpp"

#include <algorithm>
#include <vector>

#include <HelperFuncs.hpp>

#include "RobotDevices.hpp"

#include "MPU6050.h"

#include "pins-control.hpp"

// Amount of readings used to average, make it higher to get more precision but 
// program will be slower  (default:1000)
int buffer_size = 1000;
// Accelerometer error allowed, make it lower to get more precision, but may
// not converge  (default:8)
int accel_deadzone = 8;
// Gyro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)
int gyro_deadzone=1;

int16_t ax = 0, ay = 0, az = 0,
        gx = 0, gy = 0, gz = 0;

int mean_ax = 0, mean_ay = 0, mean_az = 0,
    mean_gx = 0, mean_gy = 0, mean_gz = 0;

// int state = 0;
enum State {INIT, RUNNING, FINISHED};

State state;

int ax_offset = 0, ay_offset = 0, az_offset = 0,
    gx_offset = 0, gy_offset = 0, gz_offset = 0;

void sample();
void calibration();

Serial pc(RJ_SERIAL_RXTX);


std::shared_ptr<SharedI2C> shared_i2c = make_shared<SharedI2C>(RJ_I2C_SDA, RJ_I2C_SCL, RJ_I2C_FREQ);
MPU6050 imu(shared_i2c, MPU6050_DEFAULT_ADDRESS);
LocalFileSystem local("local");

int main() {
    pc.baud(57600);

    // initialize device
    imu.initialize();

    imu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    imu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

    // start message
    printf("MPU6050 Calibration\r\n");
    printf("Place MPU6050 flat, package letters facing up. Don't touch it until all axes calibrated.\r\n");
    // verify connection
    if (imu.testConnection()) {
        printf("MPU6050 connection successful.\r\n");
    } else {
        printf("MPU6050 connection failed.\r\n");
    }

    Thread::wait(1000);

    // reset offsets
    imu.setXAccelOffset(0);
    imu.setYAccelOffset(0);
    imu.setZAccelOffset(0);
    imu.setXGyroOffset(0);
    imu.setYGyroOffset(0);
    imu.setZGyroOffset(0);

    while (true) {
        if (state == INIT){
            printf("Reading sensors for first time...\r\n");
            sample();
            state = RUNNING;
            Thread::wait(1000);
        }

        if (state == RUNNING) {
            printf("Calculating offsets...\r\n");
            calibration();
            state = FINISHED;
            Thread::wait(1000);
        }

        if (state == FINISHED) {
            sample();
            printf("FINISHED, sensor readings with offsets:\r\n");

            printf("Check that these sensor readings are close to 0 0 16384 0 0 0\r\n");
            printf("%d %d %d %d %d %d\r\n", mean_ax, mean_ay, mean_az,
                                            mean_gx, mean_gy, mean_gz);

            printf("Your offsets:\t\r\n");
            printf("%d %d %d %d %d %d\r\n", ax_offset, ay_offset, az_offset,
                                            gx_offset, gy_offset, gz_offset);

            printf("Data is printed as: accelX accelY accelZ gyroX gyroY gyroZ\r\n");

            FILE *fp = fopen("/local/offsets.txt", "w");
            if (fp != nullptr) {
                int res = fprintf(fp, "%d %d %d %d %d %d", ax_offset, ay_offset, az_offset,
                                                           gx_offset, gy_offset, gz_offset);
                fclose(fp);

                if (res > 0) printf("Offsets written to offsets.txt.");
            }

            while (1);
        }
    }
}

void sample(){
    int i = 0;
    long buff_ax = 0, buff_ay = 0, buff_az = 0,
         buff_gx = 0, buff_gy = 0, buff_gz = 0;

    const int skip_dst = 100;

    while (i < (buffer_size + skip_dst + 1)) {
        // read raw accel/gyro measurements from device
        imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // first 100 measures are discarded
        if (i > skip_dst && i  <=  (buffer_size + skip_dst)) {
            buff_ax = buff_ax + ax;
            buff_ay = buff_ay + ay;
            buff_az = buff_az + az;
            buff_gx = buff_gx + gx;
            buff_gy = buff_gy + gy;
            buff_gz = buff_gz + gz;
        }
        if (i == (buffer_size + skip_dst)) {
            mean_ax=buff_ax / buffer_size;
            mean_ay=buff_ay / buffer_size;
            mean_az=buff_az / buffer_size;
            mean_gx=buff_gx / buffer_size;
            mean_gy=buff_gy / buffer_size;
            mean_gz=buff_gz / buffer_size;
        }
        i++;
        Thread::wait(2);
    }
}

void calibration(){
    ax_offset = -mean_ax / 8;
    ay_offset = -mean_ay / 8;
    az_offset = (16384 - mean_az) / 8;

    gx_offset = -mean_gx / 4;
    gy_offset = -mean_gy / 4;
    gz_offset = -mean_gz / 4;
    while (1){
        int ready = 0;
        imu.setXAccelOffset(ax_offset);
        imu.setYAccelOffset(ay_offset);
        imu.setZAccelOffset(az_offset);

        imu.setXGyroOffset(gx_offset);
        imu.setYGyroOffset(gy_offset);
        imu.setZGyroOffset(gz_offset);

        sample();

        printf("...\r\n");

        if (abs(mean_ax) <= accel_deadzone) ready++;
        else ax_offset = ax_offset - mean_ax / accel_deadzone;

        if (abs(mean_ay) <= accel_deadzone) ready++;
        else ay_offset = ay_offset - mean_ay / accel_deadzone;

        if (abs(16384  -  mean_az) <= accel_deadzone) ready++;
        else az_offset = az_offset+(16384 - mean_az) / accel_deadzone;

        if (abs(mean_gx) <= gyro_deadzone) ready++;
        else gx_offset = gx_offset - mean_gx / (gyro_deadzone+1);

        if (abs(mean_gy) <= gyro_deadzone) ready++;
        else gy_offset = gy_offset - mean_gy / (gyro_deadzone+1);

        if (abs(mean_gz) <= gyro_deadzone) ready++;
        else gz_offset = gz_offset - mean_gz / (gyro_deadzone+1);

        printf("Number of ready axes: %d\r\n", ready);

        if (ready==6) break;
    }
}
