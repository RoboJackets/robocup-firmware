#include "bsp.h"

/* MCU support package (MSP)*/

void HAL_MspInit(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    // TODO
}

void HAL_MspDeInit(void)
{
    // TODO
}
