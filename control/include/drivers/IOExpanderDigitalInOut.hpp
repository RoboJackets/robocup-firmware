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
    IOExpanderDigitalInOut(LockedStruct<MCP23017>& mcp, MCP23017::ExpPinName pin,
                           MCP23017::PinMode mode, bool state = false)
        : _pin(pin), _mode(mode), _state(state), _mcp23017(mcp) {
    }

    void init() {
        pinMode(_mode);
        if ((_state != (bool)read()) && _mode == MCP23017::DIR_OUTPUT)
            write(_state);
    }

    void pinMode(MCP23017::PinMode mode) {
        auto lock = _mcp23017.lock();
        lock->pinMode(_pin, mode);
    }

    /// Pulls pin low if val = 0 and pulls pin high if val >= 1
    void write(int val) {
        auto lock = _mcp23017.lock();
        _state = val > 0;
        lock->writePin(val, _pin);
    }

    /// Returns 0 if pin is low, 1 if pin is high
    int read() {
        auto lock = _mcp23017.lock();
        _state = lock->readPin(_pin);
        return _state;
    }

    /// Allows the equals operator to write to a pin
    IOExpanderDigitalInOut& operator=(int value) {
        write(value);
        return *this;
    }

    /// Allows the pin to return its value like a simple integer variable
    operator int() { return read(); }

private:
    MCP23017::ExpPinName _pin;
    MCP23017::PinMode _mode;
    bool _state;
    LockedStruct<MCP23017>& _mcp23017;
};
