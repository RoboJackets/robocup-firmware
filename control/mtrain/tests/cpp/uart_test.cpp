#include "UART.hpp"

#include "mtrain.hpp"

/* UART Test */
int main() {
    UART uart(UARTBus7);
    uint8_t* received_data;
    uart.transmit(0x1D);
    uart.receive(received_data, sizeof(0x1D));
    printf("Value of received data is %d", received_data);
}