#ifndef __TIMER_H
#define __TIMER_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f7xx_hal.h"

void MX_TIM6_Init();
void MX_TIM3_Init();

void Start_TIM6();
void Stop_TIM6();
#ifdef __cplusplus
}
#endif
#endif
