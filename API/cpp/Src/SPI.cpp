#include "SPI.hpp"
#include "stm32f7xx_ll_spi.h"

// TODO: cleanup
SPI::SPI(SpiBus spiBus, std::optional<PinName> cs, int hz): chipSelect(cs) {
    GPIO_InitTypeDef  GPIO_InitStruct;

    if (chipSelect) {
        GPIO_InitStruct.Pin = chipSelect->pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        
        HAL_GPIO_Init(chipSelect->port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)1);
    }
    
    switch (spiBus) {
        case SpiBus::SpiBus2:
            spiHandle.Instance = SPI2;

            __HAL_RCC_SPI2_CLK_ENABLE();

            // Configure SPI2 SCK
            GPIO_InitStruct.Pin       = GPIO_PIN_12;
            GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
            GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            // Configure SPI2 MISO
            GPIO_InitStruct.Pin = GPIO_PIN_14;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            // Configure SPI2 MOSI
            GPIO_InitStruct.Pin = GPIO_PIN_15;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
            break;
        case SpiBus::SpiBus3:
            spiHandle.Instance = SPI3;

            __HAL_RCC_SPI3_CLK_ENABLE();

            // Configure SPI3 SCK
            GPIO_InitStruct.Pin       = GPIO_PIN_10;
            GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
            GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

            // Configure SPI3 MISO
            GPIO_InitStruct.Pin = GPIO_PIN_4;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            // Configure SPI3 MOSI
            GPIO_InitStruct.Pin = GPIO_PIN_12;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

            HAL_NVIC_SetPriority(SPI3_IRQn, 1, 0);
            HAL_NVIC_EnableIRQ(SPI3_IRQn);
            break;
        case SpiBus::SpiBus5:
            spiHandle.Instance = SPI5;

            __HAL_RCC_SPI5_CLK_ENABLE();

            // Configure SPI5 SCK
            GPIO_InitStruct.Pin       = GPIO_PIN_7;
            GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
            GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
            HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

            // Configure SPI5 MISO
            GPIO_InitStruct.Pin = GPIO_PIN_8;
            HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

            // Configure SPI5 MOSI
            GPIO_InitStruct.Pin = GPIO_PIN_9;
            HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

            HAL_NVIC_SetPriority(SPI5_IRQn, 1, 0);
            HAL_NVIC_EnableIRQ(SPI5_IRQn);
            break;
        default:
            break;
    }

    spiHandle.Init.BaudRatePrescaler = freqToPrescaler(hz);
    spiHandle.Init.Mode              = SPI_MODE_MASTER; // TODO: Be able to specify
    spiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    spiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    spiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    spiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    spiHandle.Init.NSS               = SPI_NSS_SOFT;
    spiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB; // TODO: Be able to specify?
    spiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    spiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    spiHandle.Init.CRCPolynomial     = 7;

    HAL_SPI_Init(&spiHandle);
}

SPI::~SPI() {
    if (chipSelect) {
        HAL_GPIO_DeInit(chipSelect->port, chipSelect->pin);
    }

    if (spiHandle.Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_DISABLE();

        // DeInit SPI2 SCK
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);

        // DeInit SPI2 MISO
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);

        // DeInit SPI2 MOSI
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);

        HAL_NVIC_DisableIRQ(SPI2_IRQn);
    } else if (spiHandle.Instance == SPI3) {
        __HAL_RCC_SPI3_CLK_DISABLE();

        // DeInit SPI3 SCK
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10);

        // DeInit SPI3 MISO
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);

        // DeInit SPI3 MOSI
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

        HAL_NVIC_DisableIRQ(SPI3_IRQn);
    } else if (spiHandle.Instance == SPI5) {
        __HAL_RCC_SPI5_CLK_DISABLE();

        // DeInit SPI5 SCK
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_7);

        // DeInit SPI5 MISO
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_8);

        // DeInit SPI5 MOSI
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_9);

        HAL_NVIC_DisableIRQ(SPI5_IRQn);
    }
}

uint32_t SPI::freqToPrescaler(int hz) {
    int prescale = fPCLK/hz;
    if (spiHandle.Instance == SPI5) {
        prescale = fPCLK/hz * 2;
    }
    for (int i = 1; i <= 8; i++) {
        if (prescale <= 1 << i) {
            return (i-2) << 3;
        }
    }
    return LL_SPI_BAUDRATEPRESCALER_DIV256;
}

void SPI::frequency(int hz) {
    LL_SPI_SetBaudRatePrescaler(spiHandle.Instance, freqToPrescaler(hz));
}

uint8_t SPI::transmitReceive(uint8_t data) {
    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)0);
    }

    spiHandle.Instance->CR1 |= SPI_CR1_SPE;

    /* Wait for TX queue to empty */
    while (!LL_SPI_IsActiveFlag_TXE(spiHandle.Instance)) { }
    *(__IO uint8_t *)&spiHandle.Instance->DR = data;
    /* Wait for RX queue to fill */
    while (!LL_SPI_IsActiveFlag_RXNE(spiHandle.Instance)) { }
    uint8_t recievedData = spiHandle.Instance->DR;

    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)1);
    }
    return recievedData;
}

std::vector<uint8_t> SPI::transmitReceive(const std::vector<uint8_t>& data) {
    size_t size = data.size();
    std::vector<uint8_t> dataOut(size);
    transmitReceive(data.data(), dataOut.data(), size);

    return dataOut;
}

void SPI::transmitReceive(const uint8_t* dataIn, uint8_t* dataOut, size_t size) {
    SET_BIT(spiHandle.Instance->CR2, SPI_RXFIFO_THRESHOLD);
    LL_SPI_Enable(spiHandle.Instance);

    const uint8_t* txPtr = dataIn;
    const uint8_t* txPtrEnd = txPtr + size;

    uint8_t* rxPtr = dataOut;

    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)0);
    }

    uint8_t count;
    while (txPtr < txPtrEnd) {
        count = READ_BIT(spiHandle.Instance->SR, SPI_SR_FTLVL) >> SPI_SR_FTLVL_Pos;
        count = (count > 3 - (txPtrEnd - txPtr)) ? count : 3 - (txPtrEnd - txPtr);
        switch (count) {
            case 0:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
            case 1:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
            case 2:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
        }

        count = READ_BIT(spiHandle.Instance->SR, SPI_SR_FRLVL) >> SPI_SR_FRLVL_Pos;
        switch (count) {
            case 3:
                *rxPtr++ = *(__IO uint8_t *)spiHandle.Instance->DR;
            case 2:
                *rxPtr++ = *(__IO uint8_t *)spiHandle.Instance->DR;
            case 1:
                *rxPtr++ = *(__IO uint8_t *)spiHandle.Instance->DR;
        }
    }

    while (READ_BIT(spiHandle.Instance->SR, SPI_SR_BSY) != 0) { }
    while (LL_SPI_IsActiveFlag_RXNE(spiHandle.Instance)) {
        // TODO: make sure this reads everything
        *rxPtr++ = *(__IO uint8_t *)spiHandle.Instance->DR;
    }

    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)1);
    }
}

void SPI::transmit(uint8_t data) {
    transmitReceive(data);
}

void SPI::transmit(const std::vector<uint8_t>& data) {
    transmit(data.data(), data.size());
}

void SPI::transmit(const uint8_t* data, size_t size) {
    // int size = data.size();

    SET_BIT(spiHandle.Instance->CR2, SPI_RXFIFO_THRESHOLD);
    LL_SPI_Enable(spiHandle.Instance);

    const uint8_t* txPtr = data;
    const uint8_t* txPtrEnd = txPtr + size;

    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)0);
    }

    uint8_t count;
    while (txPtr < txPtrEnd) {
        count = READ_BIT(spiHandle.Instance->SR, SPI_SR_FTLVL) >> SPI_SR_FTLVL_Pos;
        count = (count > 3 - (txPtrEnd - txPtr)) ? count : 3 - (txPtrEnd - txPtr);
        switch (count) {
            case 0:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
            case 1:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
            case 2:
                *(__IO uint8_t *)&spiHandle.Instance->DR = *txPtr++;
        }
    }

    while (READ_BIT(spiHandle.Instance->SR, SPI_SR_BSY) != 0) { }
    while (LL_SPI_IsActiveFlag_RXNE(spiHandle.Instance)) {
        uint8_t none = *(__IO uint8_t *)spiHandle.Instance->DR;
        (void)none;
    }

    if (chipSelect) {
        HAL_GPIO_WritePin(chipSelect->port, chipSelect->pin, (GPIO_PinState)1);
    }
}
