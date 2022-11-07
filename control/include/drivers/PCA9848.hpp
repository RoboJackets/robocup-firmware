
//#include <memory>
#include "I2C.hpp"
#include "LockedStruct.hpp"

class PCA9848 {

    public :
    int i2cAddress = 226;

    PCA9848(LockedStruct<I2C>& sharedI2C);

    void writeRegister(PCA9848::Register regAddress, uint16_t val);

    private:
    LockedStruct<I2C>& _i2c;
}