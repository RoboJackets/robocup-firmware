#include "mtrain.hpp"
#include <iostream>
using namespace std;

int main(void) {
  DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};
  DigitalOut output {p34};
  AnalogIn pin(adc3); //need analog pins

  output.write(true);

  while(true) {
    uint32_t adc_val = pin.read();
    printf("Raw Value %u\r\n", adc_val);
    HAL_Delay(100);
    leds[0].toggle();
    if (adc_val > 0) {
      leds[1].toggle();
      HAL_Delay(100);
      leds[1].toggle();
    }
    if (adc_val > 1024) { // // 12 bit approximation on 5v max 1024 should be about 1.25V threshold
      leds[2].toggle();
      HAL_Delay(100);
      leds[2].toggle();
    }
  }
}
