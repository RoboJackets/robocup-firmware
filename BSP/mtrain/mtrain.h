#ifndef __MTRAIN_H
#define __MTRAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include "stm32f769i_eval.h"

void SystemClock_Config(void);
void Error_Handler(void);
void MPU_Config(void);
void CPU_CACHE_Enable(void);

#ifdef __cplusplus
}
#endif

#endif