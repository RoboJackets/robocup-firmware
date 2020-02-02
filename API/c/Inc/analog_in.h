#ifndef AnalogIn
#define AnalogIn
#include "pin_defs.h"

void adc_init(adc_pin_name pin);
uint32_t read(adc_pin_name pin);
uint16_t read_u16(adc_pin_name pin);
float getValue(adc_pin_name pin);

#endif
