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

  // writing to light sesnsor enable
  // lightSensor1.writeRegister(0, 0b10000000);
  // lightSensor1.writeRegister(TSL2572::ENABLE, 0b00000011);
  // lightSensor1.readRegister(0);
  // int regVal2 = lightSensor1.readRegister(TSL2572::C1DATAH);

  lightSensor1.writeRegister(0b100000000);
  lightSensor1.readRegister();

  // while (true)
  // {
  //   printf("%d", regVal);
  // }

  // lightSensor1.readRegister(TSL2572::ENABLE)

  // while (true)
  // {
  //   // DWT_Delay(3000);
  //   printf("ENABLE: %d\n", lightSensor1.readRegister(0x00));
  //   // DWT_Delay(3000);
  // }
}
