#pragma once

#include "PinDefs.hpp"

class DigitalIn {
public:
    /** Configures GPIO pin for digital in
     *
     * @param pin Pin def external to board
     * @param pull Pin pull type
     */
    explicit DigitalIn(PinName pin, PullType pull = PullType::PullNone);

    ~DigitalIn();

    /** Read current value of pin
     *
     * @return true (high) or false (low)
     */
    volatile bool read();

    operator bool() { return read(); }

protected:
    PinName pin;
};
