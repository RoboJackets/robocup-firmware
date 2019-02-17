#include "mtrain.hpp"
#include <iostream>
using namespace std;
int main(void){
  AnalogIn pin(pf6); //need analog pins
  while (true) {
    pin.read();
    cout << "WOOOO" << endl;
  }
}
