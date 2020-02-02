#include <iostream>
#include "AnalogIn.hpp"
#include <math.h>
using namespace std;

const int REFRESH_PERIOD = 10;
const float ADC_RATIO = 3.3 / 4096;

AnalogIn::AnalogIn(ADCPinName pin) {
   ADC_Init(pin);
   __enable_irq();
   HAL_ADC_Start(&ADC_InitStruct);
}


// TODO this setup means 1 adc can go to 1 pin only and thus only 3 adc pins can be inited need to setup multi sampling indepently
void AnalogIn::ADC_Init(ADCPinName pin) {
  __HAL_RCC_ADC3_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {};
  GPIO_InitStruct.Pin = pin.pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(pin.port, &GPIO_InitStruct);

  ADC_InitStruct.Instance                   = pin.adc;
  ADC_InitStruct.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
  ADC_InitStruct.Init.Resolution            = ADC_RESOLUTION_12B;
  ADC_InitStruct.Init.ScanConvMode          = DISABLE; /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  ADC_InitStruct.Init.ContinuousConvMode    = ENABLE; /* Continuous mode enabled to have continuous conversion */
  ADC_InitStruct.Init.DiscontinuousConvMode = DISABLE; /* Parameter discarded because sequencer is disabled */
  ADC_InitStruct.Init.NbrOfDiscConversion   = 0;
  ADC_InitStruct.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Conversion start not triggered by external event */
  ADC_InitStruct.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  ADC_InitStruct.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  ADC_InitStruct.Init.NbrOfConversion       = 1;
  ADC_InitStruct.Init.DMAContinuousRequests = ENABLE;
  ADC_InitStruct.Init.EOCSelection          = DISABLE;
  //initializes the ADC given the structure that defines it
  if (HAL_ADC_Init(&ADC_InitStruct) != HAL_OK) {
      //ERROR HANDLING
  }

  ADC_ChannelConfTypeDef sConfig = {};
  sConfig.Channel       = pin.channel;
  sConfig.Rank          = 1;
  sConfig.SamplingTime  = ADC_SAMPLETIME_480CYCLES;
  sConfig.Offset        = 0;
  //configures the channel for the ADC, given the input structure
  if (HAL_ADC_ConfigChannel(&ADC_InitStruct, &sConfig) != HAL_OK) {
     //ERROR HANDLING
  }
}

AnalogIn::~AnalogIn() {

}

// Get the ADC value, does not use DMA
uint32_t AnalogIn::read () {
  return HAL_ADC_GetValue(&ADC_InitStruct);
}

uint16_t AnalogIn::read_u16 () {
  return (uint16_t) HAL_ADC_GetValue(&ADC_InitStruct);
}

float AnalogIn::getValue() {
  return (float)(HAL_ADC_GetValue(&ADC_InitStruct)) * ADC_RATIO;
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
}
