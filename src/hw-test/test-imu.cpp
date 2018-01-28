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

LocalFileSystem local("local");

int buffersize=1000;     //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone=8;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone=1;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int16_t ax, ay, az, gx, gy, gz;

int mean_ax,mean_ay,mean_az,mean_gx,mean_gy,mean_gz,state=0;
int ax_offset,ay_offset,az_offset,gx_offset,gy_offset,gz_offset;

MPU6050 accelgyro(0x68, RJ_I2C_SDA, RJ_I2C_SCL);

void meansensors();
void calibration();
void setup();
void loop();

Serial pc(RJ_SERIAL_RXTX);


int main() {
    pc.baud(57600);

    setup();

    while (true) {
        loop();
    }

}

///////////////////////////////////   SETUP   ////////////////////////////////////
void setup() {
    // initialize device
    accelgyro.initialize();

    // Thread::wait for ready
    Thread::wait(5000);

    // start message
    printf("MPU6050 Calibration Sketch\r\n");
    Thread::wait(2000);
    printf("Your MPU6050 should be placed in horizontal position, with package letters facing up. \r\nDon't touch it until you see a finish message.\r\n");
    Thread::wait(3000);
    // verify connection
    printf(accelgyro.testConnection() ? "\r\nMPU6050 connection successful" : "MPU6050 connection failed\r\n");
    Thread::wait(1000);

    // reset offsets
    accelgyro.setXAccelOffset(0);
    accelgyro.setYAccelOffset(0);
    accelgyro.setZAccelOffset(0);
    accelgyro.setXGyroOffset(0);
    accelgyro.setYGyroOffset(0);
    accelgyro.setZGyroOffset(0);
}

///////////////////////////////////   LOOP   ////////////////////////////////////
void loop() {
    if (state==0){
        printf("Reading sensors for first time...\r\n");
        meansensors();
        state++;
        Thread::wait(1000);
    }

    if (state==1) {
        printf("Calculating offsets...\r\n");
        calibration();
        state++;
        Thread::wait(1000);
    }

    if (state==2) {
        meansensors();
        printf("FINISHED!\r\n");
        printf("Sensor readings with offsets:\t\r\n");
        printf("%d %d %d %d %d %d\r\n", mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz);
        printf("Your offsets:\t\r\n");
        printf("%d %d %d %d %d %d\r\n", ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset);
        printf("Data is printed as: acelX acelY acelZ giroX giroY giroZ\r\n");
        printf("Check that your sensor readings are close to 0 0 16384 0 0 0\r\n");
        printf("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)\r\n");
        FILE *fp = fopen("/local/offsets.txt", "w");  // Open "out.txt" on the local file system for writing
        fprintf(fp, "%d %d %d %d %d %d", ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset);
        fclose(fp);
        while (1) {
            // accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
            // printf("%d %d %d %d %d %d\r\n", ax, ay, az, gx, gy, gz);
            // printf("%d\r\n", gz);
        }
    }
}

///////////////////////////////////   FUNCTIONS   ////////////////////////////////////
void meansensors(){
    long i=0,buff_ax=0,buff_ay=0,buff_az=0,buff_gx=0,buff_gy=0,buff_gz=0;

    while (i<(buffersize+101)){
        // read raw accel/gyro measurements from device
        accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        //printf("Data: %d\r\n", ax);

        if (i>100 && i<=(buffersize+100)){ //First 100 measures are discarded
            buff_ax=buff_ax+ax;
            buff_ay=buff_ay+ay;
            buff_az=buff_az+az;
            buff_gx=buff_gx+gx;
            buff_gy=buff_gy+gy;
            buff_gz=buff_gz+gz;
        }
        if (i==(buffersize+100)){
            mean_ax=buff_ax/buffersize;
            mean_ay=buff_ay/buffersize;
            mean_az=buff_az/buffersize;
            mean_gx=buff_gx/buffersize;
            mean_gy=buff_gy/buffersize;
            mean_gz=buff_gz/buffersize;
        }
        i++;
        Thread::wait(2);
    }
}

void calibration(){
    ax_offset=-mean_ax/8;
    ay_offset=-mean_ay/8;
    az_offset=(16384-mean_az)/8;

    gx_offset=-mean_gx/4;
    gy_offset=-mean_gy/4;
    gz_offset=-mean_gz/4;
    while (1){
        int ready=0;
        accelgyro.setXAccelOffset(ax_offset);
        accelgyro.setYAccelOffset(ay_offset);
        accelgyro.setZAccelOffset(az_offset);

        accelgyro.setXGyroOffsetUser(gx_offset);
        accelgyro.setYGyroOffsetUser(gy_offset);
        accelgyro.setZGyroOffsetUser(gz_offset);

        meansensors();
        printf("...\r\n");


        if (abs(mean_ax)<=acel_deadzone) ready++;
        else ax_offset=ax_offset-mean_ax/acel_deadzone;

        if (abs(mean_ay)<=acel_deadzone) ready++;
        else ay_offset=ay_offset-mean_ay/acel_deadzone;

        if (abs(16384-mean_az)<=acel_deadzone) ready++;
        else az_offset=az_offset+(16384-mean_az)/acel_deadzone;

        if (abs(mean_gx)<=giro_deadzone) ready++;
        else gx_offset=gx_offset-mean_gx/(giro_deadzone+1);

        if (abs(mean_gy)<=giro_deadzone) ready++;
        else gy_offset=gy_offset-mean_gy/(giro_deadzone+1);

        if (abs(mean_gz)<=giro_deadzone) ready++;
        else gz_offset=gz_offset-mean_gz/(giro_deadzone+1);

        int16_t gx_offset_actu = accelgyro.getXGyroOffset();
        printf("mean_gx: %d, gx_offset: %d gx_offset_actu: %d ready: %d\r\n", abs(mean_gx), gx_offset, gx_offset_actu, ready);

        if (ready==6) break;
    }
}
