#ifndef __MTRAIN_H
#define __MTRAIN_H

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


// Definitions for LED
#define LED_PORT    GPIOE
#define LED1_PIN    GPIO_PIN_4
#define LED2_PIN    GPIO_PIN_3
#define LED3_PIN    GPIO_PIN_6
#define LED4_PIN    GPIO_PIN_5

#ifdef __cplusplus
}
#endif

#endif