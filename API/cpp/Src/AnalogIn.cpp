#include <iostream>
#include "AnalogIn.hpp"
#include "DigitalOut.hpp"
#include <math.h>
using namespace std;

const int REFRESH_PERIOD = 10;

AnalogIn::AnalogIn(ADCPinName pin) {
   DMA_Init();
   ADC_Init(pin);
   __enable_irq();
   HAL_ADC_MspInit(&ADC_InitStruct);
   HAL_ADC_Start_DMA(&ADC_InitStruct, &readValue, 1);  //third argument is length, idk what that is

}


void AnalogIn::DMA_Init(){

   __HAL_RCC_DMA2_CLK_ENABLE();

   /*##-3- Configure the NVIC for DMA #########################################*/
   /* NVIC configuration for DMA transfer complete interrupt */
   HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}


void AnalogIn::ADC_Init(ADCPinName pin) {
  __HAL_RCC_ADC3_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {};
  GPIO_InitStruct.Pin = p30.pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(p30.port, &GPIO_InitStruct);

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
  if (HAL_ADC_Init(&ADC_InitStruct) != HAL_OK) {
      //ERROR HANDLING
  }

  ADC_ChannelConfTypeDef sConfig = {};
  sConfig.Channel       = ADC_CHANNEL_4;
  sConfig.Rank          = 1;
  sConfig.SamplingTime  =  ADC_SAMPLETIME_480CYCLES;
  sConfig.Offset        = 0;
  //configures the channel for the ADC, given the input structure
  if (HAL_ADC_ConfigChannel(&ADC_InitStruct, &sConfig) != HAL_OK) {
     //ERROR HANDLING
  }
}

AnalogIn::~AnalogIn() {

}



uint32_t AnalogIn::read () {
   return readValue;
}

uint16_t AnalogIn::read_u16 () {
    return (uint16_t)readValue;
}

float AnalogIn::getValue() {
  return readValue;
}

static DMA_HandleTypeDef  hdma_adc = {};

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{

  hdma_adc.Instance = DMA2_Stream0;
  hdma_adc.Init.Channel  = DMA_CHANNEL_0;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

  if (HAL_DMA_Init(&hdma_adc) != HAL_OK) {
    //ERROR HANDLING
  }
  /* Associate the initialized DMA handle to the ADC handle */
  __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
}

extern "C" {
  void DMA2_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc);
  }
}
