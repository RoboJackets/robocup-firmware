include "mtrain.h"

int main(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  uint32_t leds[4] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};

  GPIO_InitStruct.Pin = leds[0] | leds[1] | leds[2] | leds[3];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

  int id = 0;
  int forward = 1;
  while (1) {
    HAL_Delay(100);
    HAL_GPIO_WritePin(LED_PORT, leds[id], GPIO_PIN_RESET);
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

    HAL_GPIO_WritePin(LED_PORT, leds[id], GPIO_PIN_SET);
  }
}
