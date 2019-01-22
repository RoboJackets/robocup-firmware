#include "mtrain.h"
#include "analog_in.h"

int main(void) {
  char desc [50];
  printf(desc, "We are here %c", 75);
  float pin = 1;
  while (1) {
    read(pin);
  }
}
