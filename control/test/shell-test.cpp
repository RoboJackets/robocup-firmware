#include "drivers/TSL2572.hpp"
#include "drivers/PCA9858PWJ.hpp"
#include "iodefs.h"


int main()
{
    static LockedStruct<I2C> sharedI2C(SHARED_I2C_BUS);
    TSL2572 lightSensor = new TSL2572(57, 0);
    //PCA9858PWJ pca = new PCA9858PWJ(242, 0);

    auto i2c_lock = _i2c.lock();
    uint32_t values[4];
    uint8_t channel = 2;
    for (int i = 0; i < 4; i++)
    {
        //channel selector
        bool error = writeByte(226, 0, channel);

    // while(true) {
    //     printf("lux: %ld\n", lightSensor.calculateLux(1));
    // }
    return 0; 

        channel << 1;
    }
    while (true) {
        for (uint32_t value : values) {
            printf("Here's a fun value =D %d", value);
        }
    }
}
