#include "mtrain.hpp"

int main(void){
  AnalogIn pin(pf6); //need analog pins
  while (true) {
    pin.read();
  }
}
