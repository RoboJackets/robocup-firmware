#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f7xx_hal.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"
#include "qspi.h"

extern void bsp_config(void);

void SystemClock_Config(void);
void Error_Handler(void);
void MPU_Config(void);
void CPU_CACHE_Enable(void);
void DWT_Config(void);

#ifdef __cplusplus
}
#endif

#endif