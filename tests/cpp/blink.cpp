#include "mtrain.hpp"

// Tests digital out using onboard LEDs in a looping pattern
int main(void)
{
  DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};

  int id = 0;
  int forward = 1;
  while (1) {
    // Toggle Testing
    leds[id].toggle();
    HAL_Delay(100);
    leds[id].toggle();
    if (id == 3) {
        forward = -1;
    } else if(id == 0) {
        forward = 1;
    }
    id += forward;
  }
}
