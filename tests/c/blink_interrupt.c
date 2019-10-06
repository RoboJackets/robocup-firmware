#include "mtrain.h"
#include "interrupt_in.h"

/** interruptin and digitalout test
 *
 * Blinks LEDs back and forth, changing 4th LED as interrupt flag for all
 * interruptible pins
 */
void blink()
{
    digitalout_toggle(LED4);
}

int main(void)
{
    digitalout_init(LED1);
    digitalout_init(LED2);
    digitalout_init(LED3);
    digitalout_init(LED4);

    interruptin_init_ex(PIN31, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN35, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN16, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN30, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN18, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN20, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN11, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN34, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN13, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN36, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN5, &blink, PULL_DOWN, INTERRUPT_RISING);
    interruptin_init_ex(PIN4, &blink, PULL_DOWN, INTERRUPT_RISING);

    pin_name leds[] = {LED1, LED2, LED3, LED4};

    int id = 0;
    int forward = 1;

    while (1)
    {
        digitalout_toggle(leds[id]);
        HAL_Delay(100);
        digitalout_toggle(leds[id]);

        if (id == 2)
        {
            forward = -1;
        }
        else if (id == 0)
        {
            forward = 1;
        }
        id += forward;
    }
}