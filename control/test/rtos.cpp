#include "mJackets.hpp"
#include "cmsis_os.h"

static void blinkTask( void const *argument )
{
  digitalout_init(LED2);

  for(;;) {
    osDelay(500);
    digitalout_toggle(LED2);
  }
}

static void blinkTaskArg( void const *pin )
{
  digitalout_init(*(struct pin_name*)pin);

  for(;;) {
    osDelay(50);
    digitalout_toggle(*(struct pin_name*)pin);
  }
}

pin_name led3 = LED3;
pin_name led4 = LED4;

/**
 * Blink LEDs at different rates
 */
int main(void)
{

  digitalout_init(LED1);
  digitalout_write(LED1, 1);

  osThreadDef(blinkTask, blinkTask, osPriorityAboveNormal, 1, 1000);
  osThreadCreate(osThread(blinkTask), NULL);

  osThreadDef(blinkTaskArg, blinkTaskArg, osPriorityAboveNormal, 2, 1000);
  osThreadCreate(osThread(blinkTaskArg), &led3);
  osThreadCreate(osThread(blinkTaskArg), &led4);

  osKernelStart();

  for(;;);
}
