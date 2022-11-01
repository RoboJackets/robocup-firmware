#include <iostream>
using namespace std;

#include "UART.hpp"

/*
 TODO:
    - Write transmit and receive functions
    - Write recover bus function
*/
/// @brief Constructor for UART object
/// @param ub: Must specify the UART bus on the STM32f769NI that you are attempting to use
UART::UART(UARTBus ub) {
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    uartHandle.AdvancedInit.AutoBaudRateEnable =
        UART_ADVFEATURE_AUTOBAUDRATE_ENABLE;  // Enables auto baud rate detection. Alternatively, a
                                              // particular baud rate can be set.
    uartHandle.AdvancedInit.AutoBaudRateMode =
        UART_ADVFEATURE_AUTOBAUDRATE_ONSTARTBIT;  // Attempts to automatically determine baud rate
                                                  // on start bit. Can be changed.
    uartHandle.Init.WordLength =
        UART_WORDLENGTH_8B;                      // Assumes 8 bit word length. Can also be 7 or 9
    uartHandle.Init.StopBits = UART_STOPBITS_1;  // Assumes 1 UART stop bit Can also be 2
    uartHandle.Init.Parity = UART_PARITY_NONE;   // We'll set the parity to none right now
    uartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // No hardware flow control
    uartHandle.Init.Mode =
        UART_MODE_TX_RX;  // We're both transmitting and receiving with UART on this device.

    switch (ub) {
        // What radio would actually be connected to on the STM32F76

        /* Basically the "settings" for UART being used on the UART7 bus. To figure out which pins
         * correspond to what bus, see datasheet */
        case UARTBus::UARTBus7:
            uartHandle.Instance = UART7;

            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART7;
            RCC_PeriphCLKInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
            __HAL_RCC_UART7_CLK_ENABLE();

            // Configure TX pin
            GPIO_InitStruct.Pin = GPIO_PIN_7;  // Tx for UART7 is PF7
            GPIO_InitStruct.Mode =
                GPIO_MODE_AF_PP;  // stm32f7xx_hal_uart.c says to configure UART pins as alternate
                                  // function, pull up. UART is push pull for radio board.
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

            // Configure RX pin
            GPIO_InitStruct.Pin = GPIO_PIN_6;  // PF6 is TX
            // Leave the other settings in the structure the same
            HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

            break;
        // Another UART bus, not used for anything right now
        case UARTBus::UARTBus4:
            // Idk what pins these would be, would probably need to look this up
            break;
    }
    HAL_UART_Init(&uartHandle);  // Initializes UART given the previously specified settings.
                                 // Already enables device.
}

UART::~UART() {
    if (uartHandle.Instance == UART7) {
        __HAL_RCC_UART7_CLK_DISABLE();

        // DeInit UART7 Tx
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_7);

        // DeInit I2C1 SCL
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6);
    } else if (uartHandle.Instance == UART4) {
        // No case where this is applicable yet
    }
}

/// @brief Abstracted transmit function
/// @param data: The data you want to transmit
void UART::transmit(uint8_t data) {
    HAL_UART_Transmit(&uartHandle, &data, (uint16_t)sizeof(data),
                      (uint32_t)HAL_MAX_DELAY);  // TODO: Figure out a proper timeout time
}

/// @brief Abstracted receive function. TODO: Maybe? Change this to just return the data received.
/// @param pData: Pointer to a buffer where you want to store the received data
/// @param dataSize: One must specify the size of the data they are expecting to receive
void UART::receive(uint8_t* pData, uint16_t dataSize) {
    HAL_UART_Receive(&uartHandle, pData, dataSize, (uint32_t)HAL_MAX_DELAY);
}