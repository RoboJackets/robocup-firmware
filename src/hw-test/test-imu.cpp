#include "Mbed.hpp"
#include "Rtos.hpp"

#include <algorithm>
#include <vector>

#include <HelperFuncs.hpp>

#include "RobotDevices.hpp"

#include "MPU6050.h"

#include "pins-control.hpp"

bool testPass = false;
bool batchedResult = false;
std::vector<unsigned int> freq1;
std::vector<unsigned int> freq2;

int buffersize=1000;     //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone=8;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone=1;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int16_t ax = 0, ay = 0, az = 0,
        gx = 0, gy = 0, gz = 0;

int mean_ax = 0, mean_ay = 0, mean_az = 0,
    mean_gx = 0, mean_gy = 0, mean_gz = 0;

int ax_offset = 0, ay_offset = 0, az_offset = 0,
    gx_offset = 0, gy_offset = 0, gz_offset = 0;

MPU6050 imu(MPU6050_DEFAULT_ADDRESS, RJ_I2C_SDA, RJ_I2C_SCL);

Serial pc(RJ_SERIAL_RXTX);

LocalFileSystem local("local");

int main() {
    pc.baud(57600);

    printf("Starting.\r\n");

    imu.initialize();
    
    int success = 0;

    FILE *fp = fopen("/local/offsets.txt", "r");  // Open "out.txt" on the local file system for writing

    printf("open\r\n");
    if (fp != nullptr) {
        success = fscanf(fp, "%d %d %d %d %d %d", &ax_offset, &ay_offset, &az_offset,
                                                  &gx_offset, &gy_offset, &gz_offset);
        printf("fscanf done\r\n");
        fclose(fp);
        printf("closed\r\n");
    }

    imu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
    imu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

    imu.setXAccelOffset(ax_offset);
    imu.setYAccelOffset(ay_offset);
    imu.setZAccelOffset(az_offset);
    imu.setXGyroOffset(gx_offset);
    imu.setYGyroOffset(gy_offset);
    imu.setZGyroOffset(gz_offset);

    if (success == 6) {
        printf("Successfully imported offsets from offsets.txt\r\n");
    } else {
        printf("Failed to import offsets from offsets.txt, defaulting to 0\r\n");
    }

    Thread::wait(2000);

    while (true) {
        imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        printf("%d\t%d\t%d\t%d\t%d\t%d\r\n", ax, ay, az, gx, gy, gz);
        Thread::wait(2);
    }
}
