#include "mtrain.h"

int main(void)
{
  // MPU_Config();
  // CPU_CACHE_Enable();
  // HAL_Init();
  // SystemClock_Config();

  GPIO_InitTypeDef GPIO_InitStruct;
  
  uint32_t leds[4] = {GPIO_PIN_4, GPIO_PIN_3, GPIO_PIN_6, GPIO_PIN_5};

  GPIO_InitStruct.Pin = leds[0] | leds[1] | leds[2] | leds[3];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  __HAL_RCC_GPIOE_CLK_ENABLE();
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  int id = 0;
  int forward = 1;
  while (1) {
    for (int i = 0; i < 3000000; i++) {
    }
    HAL_GPIO_WritePin(GPIOE, leds[id], GPIO_PIN_RESET);
    if (id == 3) {
      forward = 0;
    } else if (id == 0) {
      forward = 1;
    }

    if (forward) {
      id++;
    } else {
      id--;
    }

    HAL_GPIO_WritePin(GPIOE, leds[id], GPIO_PIN_SET);
  }
}