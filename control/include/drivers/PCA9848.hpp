#include "I2C.hpp"
#include "LockedStruct.hpp"

class PCA9848 {
public:
    typedef enum { I2C_BUS_ADDRESS = 0xE2 } Register;
    void writeRegister(uint8_t data);

private:
    LockedStruct<I2C>& _i2c;
    PCA9848(LockedStruct<I2C>& sharedI2C);
};