#include "mtrain.hpp"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "I2C.hpp"
#include "delay.h"
#include <unistd.h>

#include "MicroPackets.hpp"
#include "iodefs.h"

#include "LockedStruct.hpp"
#include <vector>
#include <algorithm>
#include <cstdio>
#include <memory>

#include "drivers/TSL2572.hpp"
#include "drivers/PCA9858PWJ.hpp"
#include "iodefs.h"
#include "delay.h"
#include "LockedStruct.hpp"
#define CONTROL 0xE2

int main()
{
    
    

    static LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);

     while (true) {
        DWT_Delay(100);
        printf("hi\n"); 
    }

   
    //PCA9858PWJ mux(sharedI2C); 
    
    TSL2572 lightSensor(0, sharedI2C);
    //PCA9858PWJ pca = new PCA9858PWJ(242, 0);


    int i = 4;
    
    for (i = 4; i < 8; i++) {
        //mux.writeRegister(0b00010000 << (i - 4)); 
        printf("%d:\t%d\n", lightSensor.readRegister(TSL2572::C0DATA), i - 4); 
    }
    // auto i2c_lock = _i2c.lock();
    // uint32_t values[4];
    // uint8_t channel = 2;
    // for (int i = 0; i < 4; i++)
    // {
    //     //channel selector
    //     bool error = writeByte(226, 0, channel);

    // // while(true) {
    // //     printf("lux: %ld\n", lightSensor.calculateLux(1));
    // // }
    // return 0; 

    //     channel << 1;
    // }
    // while (true) {
    //     for (uint32_t value : values) {
    //         printf("Here's a fun value =D %d", value);
    //     }
    // }
}
