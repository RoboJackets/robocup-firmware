// #include "modules/RobotShellModule.hpp"

// #include "iodefs.h"



// RobotShellModule::RobotShellModule(LockedStruct<I2C>& i2c)
//     : GenericModule(kPeriod, "shell", kPriority),
//       sharedI2C(i2c) {

// }

// void RobotShellModule::entry(void) {
//     static LockedStruct<I2C> sharedI2C(I2CBus::I2CBus1);
//      while (true) {
//         HAL_Delay(100);
//         printf("hi\n"); 
//     }
//     TSL2572 lightSensor(0, sharedI2C);
//     int i = 4;
    
//     for (i = 4; i < 8; i++) {
//         //mux.writeRegister(0b00010000 << (i - 4)); 
//         printf("%d:\t%d\n", lightSensor.readRegister(TSL2572::C0DATA), i - 4); 
//     }
// }
