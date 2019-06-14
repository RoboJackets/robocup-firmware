#pragma once

#include "drivers/MCP23017.hpp"
#include <memory>

/**
 * A DigitalInOut class meant to replicate basic functionality of the
 * mBed DigitalOut and DigitalIn on the io-expander.
 */
class IOExpanderDigitalInOut {
public:
    /// Other constructors for creating objects for pinouts
    IOExpanderDigitalInOut(std::shared_ptr<MCP23017> mcp, MCP23017::ExpPinName pin,
                           MCP23017::PinMode mode, bool state = false)
        : _pin(pin), _mcp23017(mcp) {
        pinMode(mode);
        if ((state != (bool)read()) && mode == MCP23017::DIR_OUTPUT)
            write(state);
    }

    void pinMode(MCP23017::PinMode mode) { _mcp23017->pinMode(_pin, mode); }

    /// Pulls pin low if val = 0 and pulls pin high if val >= 1
    void write(int val) { _mcp23017->writePin(val, _pin); }

    /// Returns 0 if pin is low, 1 if pin is high
    int read() { return _mcp23017->readPin(_pin); }

    /// Allows the equals operator to write to a pin
    IOExpanderDigitalInOut& operator=(int value) {
        write(value);
        return *this;
    }

    /// Allows the pin to return its value like a simple integer variable
    operator int() { return read(); }

private:
    MCP23017::ExpPinName _pin;
    std::shared_ptr<MCP23017> _mcp23017;
};
