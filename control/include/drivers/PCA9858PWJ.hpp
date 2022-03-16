#include "I2C.hpp"
#define CONTROL 0xE2

class PCA9858PWJ {
    public: 
        uint16_t readRegister(); 
        void writeRegister(uint16_t data); 
        PCA9858PWJ(LockedStruct<I2C>& sharedI2C);

    

private:
    LockedStruct<I2C>& _i2c;

};