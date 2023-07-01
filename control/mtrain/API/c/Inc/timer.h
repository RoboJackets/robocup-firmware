#ifndef __TIMER_H
#define __TIMER_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f7xx_hal.h"

void MX_TIM3_Init();

/**
 *
 * @param num_callbacks this times 8 is how many seconds to wait before reset firmware
 */
void Start_TIM3(uint8_t num_callbacks);
void Stop_TIM3();
#ifdef __cplusplus
}
#endif
#endif
