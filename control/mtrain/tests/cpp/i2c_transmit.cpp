#include "I2C.hpp"
#include "mtrain.hpp"

/* I2C test using w/ blocking transmit and receive functions */
int main() {
    I2C i2c1(I2CBus1);
    fflush(stdout);
    while (true) {
        HAL_StatusTypeDef ret = i2c1.transmit(0x08, 0b00000010);
        HAL_Delay(500);
        printf("%X", ret);
        printf("\r\n");
        HAL_Delay(500);
    }
}