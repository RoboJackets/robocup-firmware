#include "mtrain.hpp"
#include <iostream>
using namespace std;
int main(void){
  DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};
  AnalogIn pin(pf6); //need analog pins
  while(true) {
    leds[0].toggle();
    if (pin.read() == 0) {
      leds[1].toggle();
      HAL_Delay(100);
      leds[1].toggle();
    }
    else if (pin.read() < 4096) { // // 12 bit approximation on 5v max 1024 should be about 1.25V threshold
      leds[2].toggle();
      HAL_Delay(100);
      leds[2].toggle();
    }
    else {
      leds[3].toggle();
      HAL_Delay(100);
      leds[3].toggle();
    }
  }
}
