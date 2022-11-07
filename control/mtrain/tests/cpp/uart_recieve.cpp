#include "UART.hpp"
#include "mtrain.hpp"

/**
 *
 */
int main(void) {
    DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};
    auto uart7 = UART(UARTBus7);

    int id = 0;
    bool forward = true;
    uint8_t data[1];
    data[0] = 0;
    while (true) {
        uart7.receive(data, 1);
        printf("data = %d", data[0]);
        leds[id].toggle();
        HAL_Delay(100);
        leds[id].toggle();

        if (id == 3) {
            forward = false;
        } else if (id == 0) {
            forward = true;
        }
        id += forward ? 1 : -1;
    }
}
