#include <iostream>
#include "AnalogIn.hpp"
using namespace std;
class AnalogIn {
   private: 
      ADC_HandleTypeDef ADC_InitStruct;
      int32_t readValue; 
      uint16_t unsignedReadValue;
   public:
      AnalogIn(PinName pin) {
         ADC_InitStruct.Instance                   = pin;
         ADC_InitStruct.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
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
	
         sConfig.Channel       = ADC_CHANNEL_0;
         sConfig.Rank          = 1;
         sConfig.SamplingTime  = ADC_SAMPLETIME_56CYCLES;
         sConfig.Offset        = 0;
         //configures the channel for the ADC, given the input structure
         HAL_ADC_ConfigChannel(&ADC_InitStruct, &sConfig); 
      }
      
      ~AnalogIn() {}

      void read () {
         HAL_ADC_Start_DMA(&ADC_InitStruct, (uint32_t*)&readValue, 1);
         while (1)
         {
            HAL_Delay(REFRESH_PERIOD);
            cout << "Current value is " << readValue << endl;
         }
      }
 
      void read_u16 () {
         HAL_ADC_Start_DMA(&ADC_InitStruct, (uint32_t*)&readValue, 1);
         while (1)
         {
            HAL_Delay(REFRESH_PERIOD);
            cout << "Current value is " << unsignedReadValue << endl;
         }
      }  
}
