#include <iostream>
#include "AnalogIn.hpp"
#include "DigitalOut.hpp"
#include <math.h>
using namespace std;

const int REFRESH_PERIOD = 10;

AnalogIn::AnalogIn(ADCPinName pin) {
    GPIO_InitTypeDef gpioInit;

    __GPIOC_CLK_ENABLE();
    __ADC1_CLK_ENABLE();

    gpioInit.Pin = GPIO_PIN_6;
    gpioInit.Mode = GPIO_MODE_ANALOG;
    gpioInit.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &gpioInit);

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);

   __HAL_RCC_ADC3_CLK_ENABLE();
   ADC_InitStruct.Instance                   = pin.port;
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
   HAL_ADC_Init(&ADC_InitStruct);

   ADC_ChannelConfTypeDef sConfig;
   sConfig.Channel       = ADC_CHANNEL_4;
   sConfig.Rank          = 1;
   sConfig.SamplingTime  =  ADC_SAMPLETIME_480CYCLES;
   sConfig.Offset        = 0;
   //configures the channel for the ADC, given the input structure
   HAL_ADC_ConfigChannel(&ADC_InitStruct, &sConfig);
   HAL_ADC_Start(&ADC_InitStruct);
   //HAL_ADCEx_CalibrationStart(&ADC_InitStruct);

}

AnalogIn::~AnalogIn() {}

void AnalogIn::read () {
   if(HAL_ADC_PollForConversion(&ADC_InitStruct, 500)==HAL_OK)
      readValue = ((float)HAL_ADC_GetValue(&ADC_InitStruct)/(1<<12));
}

void AnalogIn::read_u16 () {
    //TODO
}

void AnalogIn::stop() {
    HAL_ADC_Stop(&ADC_InitStruct);
}
float AnalogIn::getValue() {
  return readValue;
}
