/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_EXTI0_Pin GPIO_PIN_4
#define USER_EXTI0_GPIO_Port GPIOE
#define USER_EXTI0_EXTI_IRQn EXTI4_IRQn
#define USER_EXTI1_Pin GPIO_PIN_13
#define USER_EXTI1_GPIO_Port GPIOC
#define USER_EXTI1_EXTI_IRQn EXTI15_10_IRQn
#define USER_EXTI2_Pin GPIO_PIN_14
#define USER_EXTI2_GPIO_Port GPIOC
#define USER_EXTI2_EXTI_IRQn EXTI15_10_IRQn
#define IMU_I2C_SDA_Pin GPIO_PIN_0
#define IMU_I2C_SDA_GPIO_Port GPIOF
#define IMU_I2C_SCL_Pin GPIO_PIN_1
#define IMU_I2C_SCL_GPIO_Port GPIOF
#define GYRO_EXTI_Pin GPIO_PIN_2
#define GYRO_EXTI_GPIO_Port GPIOF
#define GYRO_EXTI_EXTI_IRQn EXTI2_IRQn
#define ACCEL_EXTI_Pin GPIO_PIN_3
#define ACCEL_EXTI_GPIO_Port GPIOF
#define ACCEL_EXTI_EXTI_IRQn EXTI3_IRQn
#define DIP0_Pin GPIO_PIN_7
#define DIP0_GPIO_Port GPIOF
#define DIP1_Pin GPIO_PIN_8
#define DIP1_GPIO_Port GPIOF
#define DIP2_Pin GPIO_PIN_9
#define DIP2_GPIO_Port GPIOF
#define DIP3_Pin GPIO_PIN_10
#define DIP3_GPIO_Port GPIOF
#define DIP4_Pin GPIO_PIN_0
#define DIP4_GPIO_Port GPIOC
#define DIP5_Pin GPIO_PIN_1
#define DIP5_GPIO_Port GPIOC
#define PWR_UART_TX_Pin GPIO_PIN_0
#define PWR_UART_TX_GPIO_Port GPIOA
#define PWR_UART_RX_Pin GPIO_PIN_1
#define PWR_UART_RX_GPIO_Port GPIOA
#define PWR_SRC_5V_Pin GPIO_PIN_2
#define PWR_SRC_5V_GPIO_Port GPIOA
#define PWR_SRC_3V3_Pin GPIO_PIN_3
#define PWR_SRC_3V3_GPIO_Port GPIOA
#define MOTOR_SPI_SCK_Pin GPIO_PIN_5
#define MOTOR_SPI_SCK_GPIO_Port GPIOA
#define MOTOR_SPI_MISO_Pin GPIO_PIN_6
#define MOTOR_SPI_MISO_GPIO_Port GPIOA
#define MOTOR_SPI_MOSI_Pin GPIO_PIN_7
#define MOTOR_SPI_MOSI_GPIO_Port GPIOA
#define MOTOR0_SPI_CS_Pin GPIO_PIN_5
#define MOTOR0_SPI_CS_GPIO_Port GPIOC
#define MOTOR1_SPI_CS_Pin GPIO_PIN_0
#define MOTOR1_SPI_CS_GPIO_Port GPIOB
#define MOTOR2_SPI_CS_Pin GPIO_PIN_1
#define MOTOR2_SPI_CS_GPIO_Port GPIOB
#define MOTOR3_SPI_CS_Pin GPIO_PIN_2
#define MOTOR3_SPI_CS_GPIO_Port GPIOB
#define DRIBBLER_SPI_CS_Pin GPIO_PIN_11
#define DRIBBLER_SPI_CS_GPIO_Port GPIOF
#define EXPANSION_SCL_Pin GPIO_PIN_14
#define EXPANSION_SCL_GPIO_Port GPIOF
#define EXPANSION_SDA_Pin GPIO_PIN_15
#define EXPANSION_SDA_GPIO_Port GPIOF
#define KICKER_I2C_SCL_Pin GPIO_PIN_10
#define KICKER_I2C_SCL_GPIO_Port GPIOB
#define KICKER_I2C_SDA_Pin GPIO_PIN_11
#define KICKER_I2C_SDA_GPIO_Port GPIOB
#define LED_SPI_SCK_Pin GPIO_PIN_13
#define LED_SPI_SCK_GPIO_Port GPIOB
#define LED_SPI_MOSI_Pin GPIO_PIN_15
#define LED_SPI_MOSI_GPIO_Port GPIOB
#define VBUS_SENSE_Pin GPIO_PIN_9
#define VBUS_SENSE_GPIO_Port GPIOA
#define ESP_SPI_MOSI_Pin GPIO_PIN_6
#define ESP_SPI_MOSI_GPIO_Port GPIOD
#define ESP_SPI_SCK_Pin GPIO_PIN_3
#define ESP_SPI_SCK_GPIO_Port GPIOB
#define ESP_SPI_MISO_Pin GPIO_PIN_4
#define ESP_SPI_MISO_GPIO_Port GPIOB
#define ESP_SPI_CS_Pin GPIO_PIN_5
#define ESP_SPI_CS_GPIO_Port GPIOB
#define ESP_HANDSHAKE_Pin GPIO_PIN_6
#define ESP_HANDSHAKE_GPIO_Port GPIOB
#define ESP_HANDSHAKE_EXTI_IRQn EXTI9_5_IRQn
#define ESP_DATA_READY_Pin GPIO_PIN_7
#define ESP_DATA_READY_GPIO_Port GPIOB
#define ESP_DATA_READY_EXTI_IRQn EXTI9_5_IRQn
#define OLED_I2C_SCL_Pin GPIO_PIN_8
#define OLED_I2C_SCL_GPIO_Port GPIOB
#define OLED_I2C_SDA_Pin GPIO_PIN_9
#define OLED_I2C_SDA_GPIO_Port GPIOB
#define ESP_RST_Pin GPIO_PIN_1
#define ESP_RST_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
