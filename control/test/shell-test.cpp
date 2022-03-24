#include "mtrain.hpp"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "I2C.hpp"
#include "delay.h"
#include <unistd.h>

#include "MicroPackets.hpp"
#include "iodefs.h"

#include "LockedStruct.hpp"
#include <vector>
#include <algorithm>
#include <cstdio>
#include <memory>

#include "drivers/TSL2572.hpp"
#include "drivers/PCA9858PWJ.hpp"
#include "iodefs.h"
#include "delay.h"
#include "LockedStruct.hpp"

#include "mtrain.h"
#include "cmsis_os.h"
#include <cstdlib>

#define CONTROL 0xE2

int main()
{

  I2C sharedI2C(I2CBus::I2CBus1);

  // std::shared_ptr<I2C> I2C = std::make_shared<I2C>(I2CBus, std::nullopt, 1'000'000);

  PCA9858PWJ mux(sharedI2C);
  // to initilize
  mux.writeRegister(0b11100011);
  // telling the mux which sensor we are talking to
  mux.writeRegister(0b00010000);

  TSL2572 lightSensor1(0, sharedI2C);
  // TSL2572 lightSensor2(2, sharedI2C);
  // TSL2572 lightSensor3(3, sharedI2C);
  // TSL2572 lightSensor4(4, sharedI2C);

  int i = 4;
  // writing to light sesnsor enable
  // lightSensor1.writeRegister(0x00, 0b10000000);
  lightSensor1.writeRegister(0x00, 0b00000011);

  // lightSensor1.readRegister(TSL2572::ENABLE)

  while (true)
  {
    // DWT_Delay(3000);
    printf("ENABLE: %d\n", lightSensor1.readRegister(0x00));
    // DWT_Delay(3000);
  }

  // DWT_Delay(100);
  // // printf("ID: %x\n", lightSensor1.readRegister(TSL2572::ID));
  // DWT_Delay(100);
  // printf("CO: %x\n", lightSensor1.readRegister(TSL2572::ENABLE));
  // DWT_Delay(100);
  // printf("C0H: %x\n", lightSensor1.readRegister(TSL2572::C0DATAH));
  // DWT_Delay(100);
  // printf("C1: %x\n", lightSensor1.readRegister(TSL2572::C1DATA));
  // DWT_Delay(100);
  // printf("C1H: %x\n", lightSensor1.readRegister(TSL2572::C1DATAH));
  // DWT_Delay(100);
  // for (i = 4; i < 8; i++)

  // {
  //   mux.writeRegister(0b00010000 << (i - 4));
  //   printf("%d:\t%x\n", i - 4, lightSensor1.readRegister(TSL2572::ID));
  // }

  // printf("%d:\t%d\n", lightSensor2.readRegister(TSL2572::C0DATA), 2);
  // printf("%d:\t%d\n", lightSensor3.readRegister(TSL2572::C0DATA), 3);
  // printf("%d:\t%d\n", lightSensor4.readRegister(TSL2572::C0DATA), 4);
}
