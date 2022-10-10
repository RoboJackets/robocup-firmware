#include <iostream>
using namespace std;

#include "UART.hpp"

UART::UART(UARTBus ub) {
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    switch(ub){
        // What radio would actually be connected to on the STM32F76
        case UARTBus::UARTBus7:

        // Another UART bus, not necessarily used for anything right now
        case UARTBus::UARTBus4:

    }
}