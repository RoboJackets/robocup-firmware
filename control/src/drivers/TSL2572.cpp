#include "LockedStruct.hpp"
#include "drivers/TSL2572.hpp"

// constructor
TSL2572::TSL2572(int32_t sensorID, LockedStruct<I2C>& sharedI2C)
    : _i2c(sharedI2C) {
        this->_tsl2572SensorID = sensorID; 
}

// init
// bool TSL2572::init()
// {
//     this->_tsl2572initialized = true;
//     setIntegrationTime(_tsl2572IntegrationTime);
//     return true;
// }
// begin
// bool TSL2572::begin(LockedStruct<I2C> &sharedI2C)
// {
//     this->_i2c = sharedI2C;
//     return init();
// }

// void TSL2572::reset()
// {
//     auto lock = _i2c.lock();

//     for (int reg_addr = 2; reg_addr <= OLAT; reg_addr += 2)
//         writeRegister(static_cast<TSL2572::Register>(reg_addr), 0x0000);
// }

// // init
// void TSL2572::init()
// {
//     reset(); 
// }

// 13ms
// 101ms
// 402ms
// void TSL2572::setIntegrationTime(tsl2572IntegrationTime_t time)
// {
//     this->_tsl2572IntegrationTime = time;
//     writeRegister(ATIME, time);
// }

// writes to a specific register
void TSL2572::writeRegister(TSL2572::Register regAddress, uint8_t data)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff),
                                static_cast<uint8_t>(data >> 8)};
    i2c_lock->transmit(0x39, regAddress, buffer);
}

// reads a specific register
uint16_t TSL2572::readRegister(TSL2572::Register regAddress)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = i2c_lock->receive(0x39, regAddress, 2);
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

// Commented out for now b/c relies on gain being set correctly.
/*
void TSL2572::getLuminosity(uint16_t *broadband)
{
    if (!(this->_tsl2572Gain))
    {
        getData(broadband);
        return;
    }
}
*/

/*
uint32_t TSL2572::calculateLux()
{
    unsigned long channel0;
    unsigned long channel0H; 
    // chScale = TSL2572_LUX_CHSCALE_TINT0;
    channel0 = (readRegister(C0DATA));
    channel0H = readRegister(C0DATAH); 
    return channel0;
}
*/

// write to a specific register
// void TSL2572::reset()
// {
//     auto lock = _i2c.lock();


// void TSL2572::getData(uint16_t *broadband)
// {
//     enable();
//     delay(TSL2572_DELAY_INTTME_13MS);

//     *broadband = readRegister(C0DATA);

//     disable();
// }

// void TSL2572::enable(void)
// {
//     write8(TSL2572_COMMAND_BIT | TSL2572_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
// }

// void TSL2572::disable(void)
// {
//     write8(TSL2572_COMMAND_BIT | TSL2572_REGISTER_CONTROL, TSL2572_CONTROL_POWEROFF);
// }

//try either . or * for _i2c instead of -> to fix compile error
/*
void TSL2572::write8(u_int8_t reg, u_int8_t value)
{
    auto i2c_lock = _i2c.lock();
    _i2c.beginTransmission(_i2cAddress);
    _i2c.write(reg);
    _i2c.write(value);
    _i2c.endTransmission();
}

uint8_t TSL2572::read8(uint8_t reg)
{
    _i2c.beginTransmission(_i2cAddress);
    _i2c.write(reg);
    _i2c.endTransmission();

    _i2c.requestFrom(_i2cAddress, 1); // requestFrom(address, quantity)
    return _i2c->read();
}

uint16_t TSL2572::read16(uint8_t reg)
{
    uint16_t x;
    uint16_t t;

    _i2c.beginTransmission(_i2cAddress);
    _i2c.write(reg);
    _i2c.endTransmission();

    _i2c.requestFrom(_i2cAddress, 2); // requestFrom(address, quantity)
    t = _i2c->read();
    x = _i2c->read();
    x <<= 8;
    x |= t;
    return x;
}
    */

