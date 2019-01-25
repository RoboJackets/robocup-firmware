#include "mtrain.h"

// Tests digital out using onboard LEDs in a looping pattern
int main(void)
{
  digitalout_init(LED1);
  digitalout_init(LED2);
  digitalout_init(LED3);
  digitalout_init(LED4);
  
  pin_name leds[] = {LED1, LED2, LED3, LED4};

  int id = 0;
  int forward = 1;
  while (1) {
    // Toggle Testing
    digitalout_toggle(leds[id]);
    HAL_Delay(100);
    digitalout_toggle(leds[id]);
    if (id == 3) {
        forward = -1;
    } else if(id == 0) {
        forward = 1;
    }
    id += forward;
  }
}
