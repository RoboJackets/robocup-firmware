#include "delay.h"

#include "stm32f7xx_hal.h"

uint32_t DWT_GetTick() { return DWT->CYCCNT; }

uint32_t DWT_us_To_SysTick() { return SystemCoreClock / 1000000L; }

uint64_t DWT_Get_us() { return DWT_GetTick() * DWT_us_To_SysTick(); }

void DWT_Delay_Sys(uint32_t ticks) {
    volatile uint32_t startTick = DWT->CYCCNT;
    do {
    } while (DWT->CYCCNT - startTick < ticks);
}

void DWT_Delay(uint32_t us)  // microseconds
{
    DWT_Delay_Sys(us * DWT_us_To_SysTick());
}
