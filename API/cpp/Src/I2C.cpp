#include "I2C.hpp"
#include "delay.h"

#define I2C_TIMING 0x10A60D20

I2C::I2C(I2CBus i2cBus) {
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    i2cHandle.Init.Timing           = I2C_TIMING;
    i2cHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2cHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2cHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2cHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;  
    i2cHandle.Init.OwnAddress1      = 0xFF;
    i2cHandle.Init.OwnAddress2      = 0xFF;

    switch (i2cBus) {
        case I2CBus::I2CBus1:
            i2cHandle.Instance = I2C1;

            RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
            RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
            HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
            __HAL_RCC_I2C1_CLK_ENABLE();

            // Configure SDA pin
            GPIO_InitStruct.Pin       = GPIO_PIN_7;
            GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
            GPIO_InitStruct.Pull      = GPIO_PULLUP;
            GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            // Configure SCL
            // Same pin modes as above
            GPIO_InitStruct.Pin       = GPIO_PIN_6;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            break;
        case I2CBus::I2CBus4:
            // p13/p14
            break;
    }

    HAL_I2C_Init(&i2cHandle);
    HAL_I2CEx_ConfigAnalogFilter(&i2cHandle, I2C_ANALOGFILTER_ENABLE);
}

I2C::~I2C() {
    
    if (i2cHandle.Instance == I2C1) {
        __HAL_RCC_I2C1_CLK_DISABLE();

        // DeInit I2C1 SDA
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

        // DeInit I2C1 SCL
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    } else if (i2cHandle.Instance == I2C4) {
    }
}


void I2C::recover_bus() {
    GPIO_InitTypeDef GPIO_InitStruct;

    // Revert scl back to gpio
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    GPIO_InitStruct.Pin       = GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pin       = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // todo select correct pins/port
    for (unsigned i = 0; i < 20; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PinState::GPIO_PIN_RESET);
        DWT_Delay(9); // Actually 9.54 us
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PinState::GPIO_PIN_SET);
        DWT_Delay(9);
    }

    
    // Configure SDA pin for i2c again
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
    GPIO_InitStruct.Pin       = GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void I2C::transmit(uint8_t address, uint8_t regAddr, uint8_t data) {
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&i2cHandle, address,
                                              regAddr, 1,
                                              &data, 1,
                                              5);

    // Possibly a bus error?
    // Try to do a bus recovery
    // by sending some clock cycles out
    if (ret != HAL_OK) {
        recover_bus();
    }
}

void I2C::transmit(uint8_t address, uint8_t regAddr, const std::vector<uint8_t>& data) {
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&i2cHandle, address,
                                              regAddr, 1,
                                              (uint8_t*)(data.data()), data.size(),
                                              5);

    // Possibly a bus error?
    // Try to do a bus recovery
    // by sending some clock cycles out
    if (ret != HAL_OK) {
        recover_bus();
    }
}

uint8_t I2C::receive(uint8_t address, uint8_t regAddr) {
    uint8_t data = 0;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&i2cHandle, address,
                                             regAddr, 1,
                                             &data, 1,
                                             5);

    // Possibly a bus error?
    // Try to do a bus recovery
    // by sending some clock cycles out
    if (ret != HAL_OK) {
        recover_bus();
    }

    return data;
}

std::vector<uint8_t> I2C::receive(uint8_t address, uint8_t regAddr, size_t count) {
    std::vector<uint8_t> data(count);
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&i2cHandle, address,
                                             regAddr, 1,
                                             data.data(), count,
                                             5);

    // Possibly a bus error?
    // Try to do a bus recovery
    // by sending some clock cycles out
    if (ret != HAL_OK) {
        recover_bus();
    }

    return data;
}