#include "mtrain.hpp"
#include "cmsis_os.h"
#include <cstdlib>

static void blinkTask( void const *argument )
{
  DigitalOut led2(LED2);

  for(;;) {
    osDelay(500);
    led2.toggle();
  }
}

static void blinkTaskArg( void const *pin )
{
  DigitalOut blinkPin(*(struct PinName*)pin);

  for(;;) {
    osDelay(50);
    blinkPin.toggle();
  }
}

PinName led3 = LED3;
PinName led4 = LED4;

/**
 * Blink LEDs at different rates
 */
int main(void)
{

  DigitalOut led1(LED1);
  led1.write(1);

  osThreadDef(blinkTask, blinkTask, osPriorityAboveNormal, 1, 1000);
  osThreadCreate(osThread(blinkTask), NULL);

  osThreadDef(blinkTaskArg, blinkTaskArg, osPriorityAboveNormal, 2, 1000);
  osThreadCreate(osThread(blinkTaskArg), &led3);
  osThreadCreate(osThread(blinkTaskArg), &led4);

  osKernelStart();

  for(;;);
}
