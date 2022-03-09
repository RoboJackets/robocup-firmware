#include "drivers/TSL2572.hpp"
#include "drivers/PCA9858PWJ.hpp"
#include "LockedStruct.hpp"
#include "iodefs.h"
#include "I2C.hpp"
#include "MicroPackets.hpp"

int main()
{

  
    static LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    // while (true) {
    //      printf("here2");
    // }
   
    static LockedStruct<TSL2572> ioExpander(TSL2572{sharedI2C, 0x37});
    
    // //PCA9858PWJ pca = new PCA9858PWJ(242, 0);

    // while(true) {
    //     printf("lux: %ld\n", lightSensor.calculateLux(1));
    // }
    return 0; 

}