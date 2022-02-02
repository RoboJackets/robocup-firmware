#include "LockedStruct.hpp"
#include "drivers/TSL2572.hpp"

//constructor
TSL2572::TSL2572(int i2cAddress, int32_t sensorID)
{
    _i2cAddress = i2cAddress;
    _tsl2572initialized = false;
    _tsl2572Integration = TSL2572_INTEGRATIONTIME_101MS;
    _tsl2572Gain = TSL2572_GAIN_16X;
    int32_t _tsl2572SensorID = sensorID;
}

//begin
boolean TSL2572::begin(LockedStruct<I2C> &sharedI2C)
{
    _i2c = sharedI2C;
    _i2c->begin();
    return init();
}

//init
boolean TSL2572::init()
{
    uint8_t readID = read8(ID);
    if (readID != 0x34)
    {
        return false;
    }
    _tsl2572initialized = true;

    setIntegrationTime(_tsl2572IntegrationTime);
    setGain(_tsl2572Gain);
    disable();
    return true;
}

//13ms
//101ms
//402ms
void TSL2572::setIntegrationTime(tsl2572IntegrationTime_t time)
{
    if (!_tsl2572initialized)
        begin();
    enable();

    write8(TSL2572_COMMAND_BIT | ATIME, time | _tsl2572Gain);
    _tsl2572IntegrationTime = time;
    disable();
}

void TSL2572::setGain(tsl2572Gain_t gain)
{
    if (!_tsl2572initialized)
        begin();

    enable();

    _tsl2572Gain = gain;

    disable();
}

void TSL2572::getLuminosity(uint16_t *broadband)
{
    if (!_tsl2572AutoGain)
    {
        getData(broadband);
        return;
    }
}

uint32_t TSL2572::calculateLux(uint16_t sensor)
{
    unsigned long channel0;
    chScale = TSL2572_LUX_CHSCALE_TINT0;
    channel0 = (broadband * chScale) >> TSL2561_LUX_CHSCALE;
    return channel0;
}

// write to a specific register
void TSL2572::writeRegister(TSL2572::Register regAddress, uint16_t data)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff),
                                static_cast<uint8_t>(data >> 8)};
    i2c_lock->transmit(_i2cAddress, regAddress, buffer);
}

// reads a specific register
uint16_t TSL2572::readRegister(TSL2572::Register regAddress)
{
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = i2c_lock->receive(_i2cAddress, regAddress, 2);
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

void TSL2572::reset()
{
    auto lock = _i2c.lock();

    for (int reg_addr = 2; reg_addr <= OLAT; reg_addr += 2)
        writeRegister(static_cast<MCP23017::Register>(reg_addr), 0x0000);
}

void TSL2572::getData(unint16_t *broadband)
{
    enable();
    delay(TSL2572_DELAY_INTTME_13MS);

    *broadband = readRegister(C0DATA);

    disable();
}

void TSL2572::enable(void)
{
    write8(TSL2572_COMMAND_BIT | TSL2572_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
}

void TSL2572::disable(void)
{
    write8(TSL2572_COMMAND_BIT | TSL2572_REGISTER_CONTROL, TSL2572_CONTROL_POWEROFF);
}

void TSL2572::write8(u_int8_t reg, u_int8_t value)
{
    _i2c->beginTransmission(_i2cAddress);
    _i2c->write(reg);
    _i2c->write(value);
    _i2c->endTransmission();
}

uint8_t TSL2572::read8(uint8_t reg)
{
    _i2c->beginTransmission(_i2cAddress);
    _i2c->write(reg);
    _i2c->endTransmission();

    _i2c->requestFrom(_i2cAddress, 1); //requestFrom(address, quantity)
    return _i2c->read()
}

uint16_t TSL2572::read16(uint8_t reg)
{
    uint16_t x; t;

    _i2c->beginTransmission(_i2cAddress);
    _i2c->write(reg);
    _i2c->endTransmission();

    _i2c->requestFrom(_i2cAddress, 2); //requestFrom(address, quantity)
    t = _i2c->read();
    x = _i2c->read();
    x <<= 8;
    x |= t;
    return x;