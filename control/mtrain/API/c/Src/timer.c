#include "timer.h"
TIM_HandleTypeDef htim;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htimer) {
    if (htimer->Instance == TIM3) {
        // Timer interrupt code here
        NVIC_SystemReset();
    }
}

void MX_TIM6_Init() {
    htim.Instance = TIM3;
    htim.Init.Prescaler = 0;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 80000000;  // Set timer period to 8 seconds
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    __HAL_TIM_ENABLE(&htim);
    /* Enable the TIM6 interrupt. This must execute at the lowest interrupt priority. */
    HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
        // idk
    }
}

void Start_TIM6() { HAL_TIM_Base_Start_IT(&htim); }

void Stop_TIM6() { HAL_TIM_Base_Stop_IT(&htim); }

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
void MX_TIM3_Init(void) {
    /* USER CODE BEGIN TIM3_Init 0 */

    /* USER CODE END TIM3_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM3_Init 1 */

    /* USER CODE END TIM3_Init 1 */
    htim.Instance = TIM3;
    uint32_t seconds = SystemCoreClock / (10000 / 8);
    htim.Init.Prescaler = seconds;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 0x1;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim) == HAL_OK) {
        // idk
        __HAL_TIM_ENABLE(&htim);
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK) {
        // idk
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK) {
        // idk
    }
    /* USER CODE BEGIN TIM3_Init 2 */
    HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
        // idk
    }

    /* USER CODE END TIM3_Init 2 */
}

/**
 * @brief This function handles TIM3 global interrupt.
 */
void TIM3_IRQHandler(void) {
    /* USER CODE BEGIN TIM3_IRQn 0 */

    /* USER CODE END TIM3_IRQn 0 */
    HAL_TIM_IRQHandler(&htim);
    /* USER CODE BEGIN TIM3_IRQn 1 */

    /* USER CODE END TIM3_IRQn 1 */
}
