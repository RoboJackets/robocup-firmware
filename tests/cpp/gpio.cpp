#include "mtrain.hpp"

/** DigitalIn and DigitalOut test
 * 
 * Read pins 35 & 36 and output value to LEDs
 */
int main(void)
{
  // Configure pins for digital out
  DigitalOut led1 (LED1);
  DigitalOut led2 (LED2);
    
  // Configure pins for digital in
  DigitalIn pin35 (p35, PullType::PullDown);
  DigitalIn pin36 (p36, PullType::PullDown);

  while (true) {
    // Read pins and assign values to LEDs
    led1 = pin35;
    led2 = pin36;
    HAL_Delay(10);
  }
}
