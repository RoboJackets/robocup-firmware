#include "mtrain.hpp"
#include <iostream>
using namespace std;
int main(void){
  AnalogIn pin(pf6); //need analog pins
  DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};
  while(true) {
    pin.read();
    leds[0].toggle();
    if (pin.getValue() == 0) {
      leds[1].toggle();
      HAL_Delay(100);
      leds[1].toggle();
    }
    else if (pin.getValue() < 0.08) {
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
