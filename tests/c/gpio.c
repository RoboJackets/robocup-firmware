#include "mtrain.h"

/** digitalout and digitalin test
 * 
 * Read pins 35 & 36 and output value to LEDs
 */
int main(void)
{
  // Configure pins for digital out
  digitalout_init(LED1);
  digitalout_init(LED2);
    
  // Configure pins for digital in
  digitalin_init_ex(PIN35, PULL_DOWN);
  digitalin_init_ex(PIN36, PULL_DOWN);

  while (1) {
    // Read pins and assign values to LEDs
    digitalout_write(LED1, digitalin_read(PIN35));
    digitalout_write(LED2, digitalin_read(PIN36));
    HAL_Delay(10);
  }
}
