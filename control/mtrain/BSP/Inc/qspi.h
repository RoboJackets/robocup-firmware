#ifndef __STM32F769I_EVAL_QSPI_H
#define __STM32F769I_EVAL_QSPI_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes */
#include "stm32f7xx_hal.h"

/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)


/* Pin/port definitions for QSPI */
#define QSPI_CS_PIN         GPIO_PIN_11
#define QSPI_CS_PORT        GPIOC
#define QSPI_CLK_PIN        GPIO_PIN_2
#define QSPI_CLK_PORT       GPIOB
#define QSPI_D0_PIN         GPIO_PIN_11
#define QSPI_D0_PORT        GPIOD
#define QSPI_D1_PIN         GPIO_PIN_12
#define QSPI_D1_PORT        GPIOD
#define QSPI_D2_PIN         GPIO_PIN_2
#define QSPI_D2_PORT        GPIOE
#define QSPI_D3_PIN         GPIO_PIN_13
#define QSPI_D3_PORT        GPIOD


uint8_t BSP_QSPI_Init(void);
uint8_t BSP_QSPI_DeInit(void);
uint8_t BSP_QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
uint8_t BSP_QSPI_Erase_Sector(uint32_t BlockAddress);
uint8_t BSP_QSPI_Erase_Chip(void);

uint8_t BSP_print_regs(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F769I_EVAL_QSPI_H */