#pragma once

#include "PinDefs.hpp"

class AnalogIn {
   private:
      ADC_HandleTypeDef ADC_InitStruct;
      float readValue;
      uint16_t unsignedReadValue;
   public:
      AnalogIn(ADCPinName pin);
      ~AnalogIn();
      void read();
      void read_u16();
      void stop();
      float getValue();
};
