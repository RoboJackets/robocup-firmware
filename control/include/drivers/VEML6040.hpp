#include "I2C.hpp"
#include "LockedStruct.hpp"
class VEML6040 {
public:
    typedef enum {
        CONF = 0x00,
        R_DATA = 0x08,
        G_DATA = 0x09,
        B_DATA = 0x0A,
        W_DATA = 0x0B,
    } Register;
    // 320 ms
    const double GSensitivity = 0.03146;
    const double MaxLux = 2062;
    void init();
    uint16_t readRegister(VEML6040::Register regAddress);
    // void writeRegister(VEML6040::Register regAddress, uint16_t val);
    int getR();
    int getG();
    int getB();
    int getW();

private:
    LockedStruct<I2C>& _i2c;
    int _i2cAddress;  // physical I2C address (0x10?)
    VEML6040(LockedStruct<I2C>& sharedI2C, int i2cAddress);
};