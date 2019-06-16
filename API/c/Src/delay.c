#include "delay.h"
#include "stm32f7xx_hal.h"

void DWT_Delay(uint32_t us) // microseconds
{
    volatile uint32_t delayTicks = us * (SystemCoreClock/1000000L);
    volatile uint32_t startTick = DWT->CYCCNT;

    do {} while (DWT->CYCCNT - startTick < delayTicks);
}