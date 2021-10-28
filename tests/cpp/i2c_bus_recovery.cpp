#include "mtrain.hpp"
#include "I2C.hpp"
#include "delay.h"


/* I2C test using MMA8452Q */
int main() {
    I2C i2c1(I2CBus1);
    while(true) {
        i2c1.recover_bus();

        HAL_Delay(500);
    }
}