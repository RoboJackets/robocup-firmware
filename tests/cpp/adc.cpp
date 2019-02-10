#include "mtrain.hpp"

int main(void){
  AnalogIn pin(p3); //need analog pins
  while (true) {
    pin.read();
  }
}
