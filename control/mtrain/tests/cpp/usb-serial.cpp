#include "mTrain.hpp"

int main() {
    DigitalOut l1 = DigitalOut(LED4);

    uint8_t i = 0;

    for (;;) {
        l1 = 0;

        HAL_Delay(50);

        l1 = 1;

        HAL_Delay(50);

        printf("%u\r\n", i++);
    }
}
