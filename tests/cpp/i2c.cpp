#include "mtrain.hpp"
#include "I2C.hpp"

#define CTRL_REG1  0x2A

/* I2C test using MMA8452Q */
int main() {
    I2C i2c1(I2CBus1, 0x1D);

    auto c = i2c1.receive(CTRL_REG1);
    i2c1.transmit(CTRL_REG1, c | 0x01);

    fflush(stdout);
    while(true) {
        auto data = i2c1.receive(0x01, 6);
        for (auto i : data) {
            printf("%d  ", i);
        }
        printf("\r\n");
        HAL_Delay(500);
    }
}