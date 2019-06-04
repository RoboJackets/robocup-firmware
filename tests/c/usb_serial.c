#include "mtrain.h"

int main(void) {

    digitalout_init(LED1);
    digitalout_init(LED2);
    digitalout_init(LED3);
    digitalout_init(LED4);
    
    pin_name leds[] = {LED1, LED2, LED3, LED4};

    fflush(stdout);

    for (;;) {
        digitalout_toggle(leds[0]);
        HAL_Delay(5);
        digitalout_toggle(leds[1]);
        HAL_Delay(5);
        digitalout_toggle(leds[2]);
        HAL_Delay(5);
        
        digitalout_toggle(leds[0]);
        HAL_Delay(5);
        digitalout_toggle(leds[1]);
        HAL_Delay(5);
        digitalout_toggle(leds[2]);
        HAL_Delay(5);

        printf("t\r\n");
        printf("a\r\n");
        fflush(stdout);
    }
}