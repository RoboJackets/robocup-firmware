#include "I2C.hpp"
#include "mtrain.hpp"

int main() {
    I2C i2c1(I2CBus1);
    fflush(stdout);
    while (true) {
        auto data = i2c1.slaveReceive();
        printf("%d  ", data);
        printf("\r\n");
        HAL_Delay(500);
    }
}