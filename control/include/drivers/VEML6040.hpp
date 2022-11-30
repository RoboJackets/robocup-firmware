
//#include <memory>?
#include "I2C.hpp"
#include "LockedStruct.hpp"

class VEML6040 {
public:
    typedef enum {
        RED = 0x08,
        GREEN = 0x09,
        BLUE = 0x0A,
        WHITE = 0x0B
    }CommandCode;

    int i2cAddress = 0; //what is the i2cAddress supposed to be?????? Bernardo should know
    
    VEML6040(LockedStruct<I2C>& sharedI2C);

    //read command that reads the value of Red Green and Blue
    uint16_t read();
    //read command that will take in a specific code to only read a certain color
    uint16_t read(VEML::CommandCode);
    void write(std::vector uint8_t data);
    void init();

private:
    LockedStruct<I2C>& _i2c;
    int _i2cAddress;
}