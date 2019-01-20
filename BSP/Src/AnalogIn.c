#include "mtrain.h"
//structure to initialize ADC
ADC_HandleTypeDef ADC_InitStruct;

__IO int32_t readValue = 0;
__IO uint16_t unsignedReadValue = 0;
REFRESH_PERIOD = 10;

int main(void) {
  //used for sprintf later
  char desc[50];

  //Idk what the CPU cache is, but enables ICache and DCache, whatever those are
  CPU_CACHE_Enable();

  //initializes the HAL
  HAL_Init();

  //Initializes system clock
  SystemClock_Config();

  //configures the Analog to Digital Conversion
  ADC_Config();
}

//reads in a float
//currently prints the value
void read(float pin)
  HAL_ADC_Start_DMA(&ADC_InitStruct, (uint32_t*)&ConvertedValue, 1);
  while (1)
  {
    /* Insert a delay define on REFRESH_PERIOD */
    HAL_Delay(REFRESH_PERIOD);

    /* Display the Value */
    sprintf(desc, "value is %ld", unsignedReadValue);
  }

}

//read in unsigned 16 bit Value
//currently just prints the value
void read_u16(unsigned short pin) {

   HAL_ADC_Start_DMA(&ADC_InitStruct, &ConvertedValue, 1);
   while (1)
  {
    /* Insert a delay define on REFRESH_PERIOD */
    HAL_Delay(REFRESH_PERIOD);

    /* Display the  Value */
    sprintf(desc, "value is %ld", readValue);
;
   // HAL_ADC_Stop();
}

//configures the clock (obviously)
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct; //clock init structure
  RCC_OscInitTypeDef RCC_OscInitStruct; //oscillator init structure
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState        = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM        = 25;
  RCC_OscInitStruct.PLL.PLLN        = 432;
  RCC_OscInitStruct.PLL.PLLP        = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ        = 9;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType       = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider  = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

//configures the ADC and the channel
static void ADC_Config(void) {
  //structure for initializing ADC channel
  ADC_ChannelConfTypeDef sConfig;

  ADC_InitStruct.Instance                   = ADC1 || ADC2 || ADC3;
  ADC_InitStruct.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
  ADC_InitStruct.Init.Resolution            = ADC_RESOLUTION_12B;
  ADC_InitStruct.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  ADC_InitStruct.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode enabled to have continuous conversion */
  ADC_InitStruct.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  ADC_InitStruct.Init.NbrOfDiscConversion   = 0;
  ADC_InitStruct.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start not trigged by an external event */
  ADC_InitStruct.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  ADC_InitStruct.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  ADC_InitStruct.Init.NbrOfConversion       = 1;
  ADC_InitStruct.Init.DMAContinuousRequests = ENABLE;
  ADC_InitStruct.Init.EOCSelection          = DISABLE;
  //initializes the ADC given the structure that defines it
  HAL_ADC_Init(ADC_InitStruct);

  sConfig.Channel       = ADC_IN3; //huh
  sConfig.Rank          = 1;
  sConfig.SamplingTime  = ADC_SAMPLETIME_56CYCLES;
  sConfig.Offset        = 0;
  //configures the channel for the ADC, given the input structure
  HAL_ADC_ConfigChannel(&ADC_InitStruct, sConfig)
}

static void CPU_CACHE_Enable(void) {
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}
