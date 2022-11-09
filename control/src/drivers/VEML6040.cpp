

#include "LockedStruct.hpp"
#include "drivers/VEML4060.hpp"


//constructor
VEML6040::VEML6040(LockedStruct<I2C>& sharedI2C, ) 
    : _i2c(sharedI2C)
{
    
}

void VEML6040::init() {
    auto i2c_lock = _i2c.lock();

    /*
    whenever initilizing we want to put 00100000
    bits 4, 5, 6 refer to integration time setting, this is set to a mid level time setting
    bit 2 is trigger, this has no trigger and its on auto mode (bit 1)
    Command Code is just 0 and data byte low is said above and data byte high is 0 by default
    */
    writeRegister({0x00, 0b00100000, 0x00});
}

uint16_t read(VEML::CommandCode) {
    auto i2c_lock = _i2c.lock();

    //new receive function being written to get the red green and blue values

    
}

//Vector of three bits, 1st: command code, 2nd: data byte low, 3rd: data byte high
void VEML6040::write(std::vector<uint8_t> data) {
    auto i2c_lock = _i2c.lock();

    //new transmit function should be written with just i2cAddress and 3 part vector for data
    i2c_lock->transmit(_i2cAddress, data);
}

/*init
bool VEML6040::init() {
    this->_i2c = sharedI2C;
    return init();
}*/