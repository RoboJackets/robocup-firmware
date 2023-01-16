#include "I2C.hpp"
#include "mtrain.hpp"

/* I2C test using w/ blocking transmit and receive functions */
int main() {
    I2C i2c1(I2CBus1);
    while (true) {
        HAL_StatusTypeDef ret = i2c1.transmit(0x04, 2);
        HAL_Delay(500);
        printf("Transmitting values");
        printf("\r\n");
        if (ret == 0x00) {
            printf("HAL_OK");
        } else if (ret == 0x01) {
            printf("HAL_ERROR");
        } else if (ret == 0x02) {
            printf("HAL_BUSY");
        } else {
            printf("TIMEOUT");
        }
        printf("\r\n");
        HAL_Delay(500);
    }
}