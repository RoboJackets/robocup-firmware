#include "timer.h"
TIM_HandleTypeDef htim;
int skip_initial_callback = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htimer) {
    if (htimer->Instance == TIM3) {
        // Timer interrupt code here
        if (skip_initial_callback++ >= 1) {
            NVIC_SystemReset();
        }
    }
}

void Start_TIM3() {
    __HAL_TIM_SET_COUNTER(&htim, 0);
    HAL_TIM_Base_Start_IT(&htim);
}

void Stop_TIM3() { HAL_TIM_Base_Stop_IT(&htim); }

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
    // these numbers are based on 8 seconds delay
    htim.Init.Prescaler = 41648;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 415999;
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
    if (HAL_TIM_Base_Init(&htim) != HAL_OK) {
        // idk
    }
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    /* USER CODE END TIM3_Init 2 */
}

/**
 * @brief This function handles TIM3 global interrupt.
 */
void TIM3_IRQHandler(void) {
    /* USER CODE BEGIN TIM3_IRQn 0 */

    /* USER CODE END TIM3_IRQn 0 */
    // printf("Timer2");
    HAL_TIM_IRQHandler(&htim);
    /* USER CODE BEGIN TIM3_IRQn 1 */

    /* USER CODE END TIM3_IRQn 1 */
}
