#include "mtrain.h"

int main(void) {
  digitalout_init(LED1);
  digitalout_init(LED2);
  digitalout_init(LED3);
  digitalout_init(LED4);

  pin_name leds[] = {LED1, LED2, LED3, LED4};

  adc_handle handle;
  analogin_init(adc3, &handle);

  while(1) {
    uint32_t adc_val = analogin_read(&handle);
    HAL_Delay(100);
    printf("%u\r\n", adc_val);
    digitalout_toggle(leds[0]);
    if (adc_val > 0) {
      digitalout_toggle(leds[1]);
      HAL_Delay(100);
      digitalout_toggle(leds[1]);
    }
    if (adc_val > 1024) { // // 12 bit approximation on 5v max 1024 should be about 1.25V threshold
      digitalout_toggle(leds[2]);
      HAL_Delay(100);
      digitalout_toggle(leds[2]);
    }
  }
}
