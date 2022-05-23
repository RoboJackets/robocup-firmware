#include <cstdlib>

#include "cmsis_os.h"
#include "common.hpp"

static void blinkTask( void const *argument )
{
    DigitalOut l2(LED2);

    for (;;) {
        osDelay(1000);
        l2.toggle();
  }
}

static void blinkTaskArg( void const *pin )
{
    DigitalOut l(*(struct PinName*)pin);

    for (;;) {
        osDelay(500);
        l.toggle();
  }
}

PinName led3 = LED3;
PinName led4 = LED4;

/**
 * Blink LEDs at different rates
 */
int main(void)
{
    DigitalOut l1(LED1);
    l1.write(1);

    osThreadDef(blinkTask, blinkTask, osPriorityAboveNormal, 1, 1000);
    osThreadCreate(osThread(blinkTask), NULL);

    osThreadDef(blinkTaskArg, blinkTaskArg, osPriorityAboveNormal, 2, 1000);
    osThreadCreate(osThread(blinkTaskArg), &led3);
    osThreadCreate(osThread(blinkTaskArg), &led4);

    osKernelStart();

    for (;;)
        ;
}
