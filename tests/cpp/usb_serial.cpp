#include "mtrain.hpp"

#include "PinDefs.hpp"

int main() {
    DigitalOut l1 = DigitalOut(LED2);

    fflush(stdout);

    uint8_t i = 0;

    for (;;) {
        l1 = 0;
        
        HAL_Delay(50);
        
        l1 = 1;

        HAL_Delay(50);

        printf("%u\r\n", i++);
        fflush(stdout);
    }
}
