#include "analog_in.h"

ADC_TypeDef adcs = [, ADC2, ADC3];
ADC_HandleTypeDef ADC_InitStruct;

__IO int32_t readValue = 0;
__IO uint16_t unsignedReadValue = 0;
REFRESH_PERIOD = 10;
//used for sprintf later
char desc[50];

void init(float pin) {
  //configures the Analog to Digital Conversion
  ADC_Config(pin);
}

//reads in a float
//currently prints the value
void read(float pin) {
  init(pin);
  HAL_ADC_Start_DMA(&ADC_InitStruct, (uint32_t*)&unsignedReadValue, 1);
  while (1)
  {
    HAL_Delay(REFRESH_PERIOD);
    sprintf(desc, "value is %ld", unsignedReadValue);
  }

}

//read in unsigned 16 bit Value
//currently just prints the value
void read_u16(unsigned short pin) {
   init(pin);
   HAL_ADC_Start_DMA(&ADC_InitStruct, &readValue, 1);
   while (1)
   {
    HAL_Delay(REFRESH_PERIOD);
    sprintf(desc, "value is %ld", readValue);
   // HAL_ADC_Stop();
   }
}



//configures the ADC and the channel
static void ADC_Config(float pin) {
  //structure for initializing ADC channel
  ADC_ChannelConfTypeDef sConfig;

  ADC_InitStruct.Instance                   = adcs[pin];
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

  sConfig.Channel       = ADC_IN3;
  sConfig.Rank          = 1;
  sConfig.SamplingTime  = ADC_SAMPLETIME_56CYCLES;
  sConfig.Offset        = 0;
  //configures the channel for the ADC, given the input structure
  HAL_ADC_ConfigChannel(&ADC_InitStruct, sConfig)
}
