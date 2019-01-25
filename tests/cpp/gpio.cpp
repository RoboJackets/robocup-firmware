#include "mtrain.hpp"

// Tests digital in using input on pins 33-36 and digital out using onboard LEDs
int main(void)
{
  // Digital Out
  // Basic Init Testing
  DigitalOut led1 (LED1);
  DigitalOut led2 (LED2);
  DigitalOut led3 (LED3);
  DigitalOut led4 (LED4);
    
  // Digital In
  // Basic Init Testing
  DigitalIn pin36 (p36, PullType::PullDown);
  DigitalIn pin35 (p35, PullType::PullDown);
  DigitalIn pin34 (p34, PullType::PullDown);
  DigitalIn pin33 (p33, PullType::PullDown);
  while (1) {
    // Read Testing
    led1.write(pin36.read());
    led2.write(pin35.read());
    led3.write(pin34.read());
    led4.write(pin33.read());
    HAL_Delay(10);
  }
}
