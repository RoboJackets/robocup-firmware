#include "mtrain.h"

// Tests digital in using input on pins 33-36 and digital out using onboard LEDs
int main(void)
{
  // Digital Out
  // Basic Init Testing
  digitalout_init(LED1);
  digitalout_init(LED2);
  digitalout_init(LED3);
  digitalout_init(LED4);
    
  // Digital In
  // Basic Init Testing
  digitalin_init_ex(PIN36, PULL_DOWN);
  digitalin_init_ex(PIN35, PULL_DOWN);
  digitalin_init_ex(PIN34, PULL_DOWN);
  digitalin_init_ex(PIN33, PULL_DOWN);
  while (1) {
    // Read Testing
    digitalout_write(LED1, digitalin_read(PIN36));
    digitalout_write(LED2, digitalin_read(PIN35));
    digitalout_write(LED3, digitalin_read(PIN34));
    digitalout_write(LED4, digitalin_read(PIN33));
    HAL_Delay(10);
  }
}
