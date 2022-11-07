//Im assuming I need include statements, don't know what needs to be included
#include "LockedStruct.hpp"
//#include "drivers/VEML4060.hpp"


//constructor
VEML6040::VEML6040(int32_t sensorID, I2C &sharedI2C) 
    : _i2c(sharedI2C)
{
    this->_veml6040SensorID = sensorID;
}

//init
bool VEML6040::init() {
    this->_i2c = sharedI2C;
    return init();
}

