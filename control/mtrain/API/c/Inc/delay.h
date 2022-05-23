#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include  <unistd.h>

/**
 * Current tick in sysclock ticks
 */
uint32_t DWT_GetTick();

uint64_t DWT_Get_us();

uint32_t DWT_us_To_SysTick();

void DWT_Delay_SysTick(uint32_t ticks);

/**
 * Delay routine itself.
 * Time is in microseconds (1/1000000th of a second), not to be
 * confused with millisecond (1/1000th).
 *
 * @param uint32_t us  Number of microseconds to delay for
 */
void DWT_Delay(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif
