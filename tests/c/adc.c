
#include "analog_in.h"

int main(void) {
  char desc [50];
  sprintf(desc, "We are here %c", 75);
  adc_pin_name pin = ADCPINF6;
  while (1) {
    read(pin);
  }
}
