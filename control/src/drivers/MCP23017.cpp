#include "drivers/MCP23017.hpp"

MCP23017::MCP23017(LockedStruct<I2C>& sharedI2C, int i2cAddress)
    : _i2c(sharedI2C), _i2cAddress(i2cAddress) {
}

void MCP23017::init() {
    reset();
}

void MCP23017::reset() {
    // Lock is a recursive mutex, so it's okay that child also call it.
    auto lock = _i2c.lock();

    // Set all pins to input mode (via IODIR register)
    inputOutputMask(0xFFFF);

    // set all other registers to zero (last of 10 registers is OLAT)
    for (int reg_addr = 2; reg_addr <= OLAT; reg_addr += 2)
        writeRegister(static_cast<MCP23017::Register>(reg_addr), 0x0000);

    // reset cached values
    _cachedGPIO = 0;
    _cachedGPPU = 0;
    _cachedIPOL = 0;
}

void MCP23017::writeRegister(MCP23017::Register regAddress, uint16_t data) {
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer{static_cast<uint8_t>(data & 0xff),
                                static_cast<uint8_t>(data >> 8)};
    i2c_lock->transmit(_i2cAddress, regAddress, buffer);
}

uint16_t MCP23017::readRegister(MCP23017::Register regAddress) {
    auto i2c_lock = _i2c.lock();
    std::vector<uint8_t> buffer = i2c_lock->receive(_i2cAddress, regAddress, 2);

    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

void MCP23017::writePin(int value, MCP23017::ExpPinName pin) {
    if (value == 0) {
        _cachedGPIO &= ~(1 << pin);
    } else {
        _cachedGPIO |= 1 << pin;
    }

    digitalWordWrite(_cachedGPIO);
}

void MCP23017::writeMask(uint16_t data, uint16_t mask) {
    _cachedGPIO = (_cachedGPIO & ~mask) | data;
    digitalWordWrite(_cachedGPIO);
}

uint8_t MCP23017::readPin(MCP23017::ExpPinName pin) {
    _cachedGPIO = digitalWordRead();

    //LOG(DEBUG,
    //    "Read an I/O pin bit:"
    //    "    Bit:\t%u\r\n"
    //    "    State:\t%s",
    //    pin, ((_cachedGPIO >> pin) & 0x0001) ? "ON" : "OFF");

    return ((_cachedGPIO >> pin) & 0x0001);
}

void MCP23017::config(uint16_t dir_config, uint16_t pullup_config,
                      uint16_t polarity_config) {
    inputOutputMask(dir_config);
    internalPullupMask(pullup_config);
    inputPolarityMask(polarity_config);

    //LOG(DEBUG,
    //    "IO Expander Configuration:\r\n"
    //    "    IODIR:\t0x%04X\r\n"
    //    "    GPPU:\t0x%04X\r\n"
    //    "    IPOL:\t0x%04X",
    //    _cachedIODIR, _cachedGPPU, _cachedIPOL);
}

void MCP23017::pinMode(ExpPinName pin, PinMode mode) {
    if (mode == DIR_INPUT) {
        _cachedIODIR |= 1 << pin;
    } else {
        _cachedIODIR &= ~(1 << pin);
    }

    inputOutputMask(_cachedIODIR);
}

int MCP23017::digitalRead(ExpPinName pin) {
    _cachedGPIO = readRegister(GPIO);
    return ((_cachedGPIO & (1 << pin)) ? 1 : 0);
}

void MCP23017::digitalWrite(ExpPinName pin, int val) {
    // If this pin is an INPUT pin, a write here will
    // enable the internal pullup
    // otherwise, it will set the OUTPUT voltage
    // as appropriate.
    bool isOutput = !(_cachedIODIR & 1 << pin);

    if (isOutput) {
        // This is an output pin so just write the value
        if (val)
            _cachedGPIO |= 1 << pin;
        else
            _cachedGPIO &= ~(1 << pin);

        digitalWordWrite(_cachedGPIO);
    } else {
        // This is an input pin, so we need to enable the pullup
        if (val) {
            _cachedGPPU |= 1 << pin;
        } else {
            _cachedGPPU &= ~(1 << pin);
        }

        internalPullupMask(_cachedGPPU);
    }
}

uint16_t MCP23017::digitalWordRead() {
    _cachedGPIO = readRegister(GPIO);
    return _cachedGPIO;
}

void MCP23017::digitalWordWrite(uint16_t w) {
    _cachedGPIO = w;
    writeRegister(GPIO, w);
}

void MCP23017::inputPolarityMask(uint16_t mask) {
    _cachedIPOL = mask;
    writeRegister(IPOL, _cachedIPOL);
}

void MCP23017::inputOutputMask(uint16_t mask) {
    _cachedIODIR = mask;
    writeRegister(IODIR, _cachedIODIR);
}

void MCP23017::internalPullupMask(uint16_t mask) {
    _cachedGPPU = mask;
    writeRegister(GPPU, _cachedGPPU);
}
