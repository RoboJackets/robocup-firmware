// #include "modules/RobotShellModule.hpp"
// #include "iodefs.h"

// RotaryDialModule::RotaryDialModule(){
// }

// void RotaryDialModule::start() {
//     shell.init();
// }

// void RotaryDialModule::entry(void) {
//     TSL2572 lightSensor = new TSL2572(57, 0);
//     //PCA9858PWJ pca = new PCA9858PWJ(242, 0);

//     auto i2c_lock = _i2c.lock();
//     uint32_t values[4];
//     uint8_t channel = 2;
//     for (int i = 0; i < 4; i++)
//     {
//         //channel selector
//         bool error = writeByte(226, 0, channel);

//         //reading from light sensor
//         values[i] = lightSensor.caculateLux(0);

//         channel << 1;
//     }
//     while (true) {
//         for (uint32_t value : values) {
//             printf("Here's a fun value =D %d", value);
//         }
//     }
// }
