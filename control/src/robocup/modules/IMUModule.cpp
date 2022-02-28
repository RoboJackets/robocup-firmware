#include "modules/IMUModule.hpp"
#include "mtrain.hpp"
#include "delay.h"
#include <cstdio>
#include "stm32f7xx_hal.h"
int IMUcount; 
 uint64_t IMUstartTime;


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

    printf("[INFO] IMU initialized\r\n");
    imuData.lock()->initialized = true;
  
   
    
}

void IMUModule::entry(void) {
    IMUstartTime =  DWT_GetTick();
    //DWT_Delay(2000);
    
    float rate = imu.gyro_z();

    auto imuDataLock = imuData.lock();
    imuDataLock->isValid = true;
    imuDataLock->lastUpdate = xTaskGetTickCount();

    imuDataLock->omegas[2] = rate; 
    if (IMUcount % 1000==0) {
        printf("IMU Time Elapsed: %f\r\n", ((DWT_GetTick()) - IMUstartTime) / 216.0f);
       
        
    }
    IMUcount++; 
}
