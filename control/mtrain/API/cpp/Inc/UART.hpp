#pragma once

#include <vector>

#include "PinDefs.hpp"

/*
    UART "buses", used to differentiate between instances in which we use UART
    For example, we may be using UART for both radio and robot shell ID, and we
    want to be able to specify which device we are communicating with. They'll hold
    unique data for transmission with that device, like which pins the
*/
typedef enum UARTBus {
    UARTBus1,
    UARTBus2,
} UARTBus;

// PinDefs includes the HAl Configuration File, which in turn includes the UART HAL file
class UART {
public:
    UART(UARTBus ub);  // Constructor
    ~UART();           // Destructor

    void transmit(uint8_t data);  // Abstracted transmit function
    void receive();               // Abstracted receive function

    void recoverBus();  // This will be used to reset things in case something goes wrong during a
                        // transmission
private:
    UART_HandleTypeDef uartHandle = {};  // Type is specified by stm32f7xx_hal_uart.h
                                         // Essentially will store all configuration, received data,
                                         // transmitted data etc. obtained during its lifetime
};