#include "mtrain.h"

/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)

uint8_t QSPI_Init(void);

QSPI_HandleTypeDef QSPIHandle;

int main(void)
{
  QSPI_Init();

}


uint8_t QSPI_Init(void)
{ 
  QSPIHandle.Instance = QUADSPI;

  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK) {
    return QSPI_ERROR;
  }
        
  GPIO_InitTypeDef gpio_init_structure;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable the QuadSPI memory interface clock */
  __HAL_RCC_QSPI_CLK_ENABLE();
  /* Reset the QuadSPI memory interface */
  __HAL_RCC_QSPI_FORCE_RESET();
  __HAL_RCC_QSPI_RELEASE_RESET();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* QSPI CS GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_CS_PIN;
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_CS_PORT, &gpio_init_structure);

  /* QSPI CLK GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_CLK_PIN;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_CLK_PORT, &gpio_init_structure);

  /* QSPI D0 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_D0_PIN;
  gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_D0_PORT, &gpio_init_structure);

  /* QSPI D1 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_D1_PIN;
  gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_D1_PORT, &gpio_init_structure);

  /* QSPI D2 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_D2_PIN;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D2_PORT, &gpio_init_structure);

  /* QSPI D3 GPIO pin configuration  */
  gpio_init_structure.Pin       = QSPI_D3_PIN;
  gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D3_PORT, &gpio_init_structure);

  /*##-3- Configure the NVIC for QSPI #########################################*/
  /* NVIC configuration for QSPI interrupt */
  HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(QUADSPI_IRQn);


  
  // /* QSPI initialization */
  // /* QSPI freq = SYSCLK /(1 + ClockPrescaler) = 216 MHz/(1+1) = 108 Mhz */
  // QSPIHandle.Init.ClockPrescaler     = 1; 
  // QSPIHandle.Init.FifoThreshold      = 4;
  // QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  // QSPIHandle.Init.FlashSize          = POSITION_VAL(N25Q512A_FLASH_SIZE) - 1;
  // QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_6_CYCLE; /* Min 50ns for nonRead */
  // QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;
  // QSPIHandle.Init.FlashID            = QSPI_FLASH_ID_1;
  // QSPIHandle.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

  // if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
  // {
  //   return QSPI_ERROR;
  // }

  // /* QSPI memory reset */
  // if (QSPI_ResetMemory(&QSPIHandle) != QSPI_OK)
  // {
  //   return QSPI_NOT_SUPPORTED;
  // }
 
  // /* Set the QSPI memory in 4-bytes address mode */
  // if (QSPI_EnterFourBytesAddress(&QSPIHandle) != QSPI_OK)
  // {
  //   return QSPI_NOT_SUPPORTED;
  // }
 
  // /* Configuration of the dummy cycles on QSPI memory side */
  // if (QSPI_DummyCyclesCfg(&QSPIHandle) != QSPI_OK)
  // {
  //   return QSPI_NOT_SUPPORTED;
  // }
  
  return QSPI_OK;
}