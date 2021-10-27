#pragma once

#include "PinDefs.hpp"

class AnalogIn {
   private:

      ADC_HandleTypeDef ADC_InitStruct = {};
      uint32_t readValue;
      uint16_t unsignedReadValue;
      void ADC_Init(ADCPinName pin);

   public:
      AnalogIn(ADCPinName pin);
      ~AnalogIn();

      uint32_t read();
      uint16_t read_u16();
      float getVoltage();
      float getPercent();
};
