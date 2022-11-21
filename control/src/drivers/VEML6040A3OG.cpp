#include "LockedStruct.hpp"
#include "drivers/VEML6040A3OG.hpp"
VEML6040A3OG::VEML6040A3OG(int32_t sensorID, I2C &sharedI2C)
    :_VEML6040A3OGSensorID(sensorID), _i2c(sharedI2C)
{
    this->_VEML6040A3OGSensorID=sensorID;
}
void VEML6040A3OG::writeRegister(uint8_t data)
{
    _i2c.transmit(0xE2h,data);
}
uint8_t VEML6040A3OG::readRegister()
{
    uint8_t buffer;
}