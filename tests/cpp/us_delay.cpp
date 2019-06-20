#include "mtrain.hpp"
#include "delay.hpp"

/**
 * Blink LEDs back and forth
 */
int main(void)
{
  DigitalOut leds[] = {(LED1), (LED2), (LED3), (LED4)};

  int id = 0;
  bool forward = true;

  while (true) {
    leds[id].toggle();
    DWT_Delay(100000);
    leds[id].toggle();

    if (id == 3) {
        forward = false;
    } else if (id == 0) {
        forward = true;
    }
    id += forward ? 1 : -1;
  }
}
