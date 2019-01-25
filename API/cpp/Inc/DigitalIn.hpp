#pragma once

#include "PinDefs.hpp"

class DigitalIn {
public:
    /**
    * Initializes Digital In
    *
    * @param PinName pin
    * @default PullType PULL_NONE
    */
    DigitalIn(PinName pin, PullType pull = PullType::PullNone);
    
    /**
    * Deinitalizes Digital In
    */
    ~DigitalIn();
    
    /**
    * Returns pin state
    *
    * @return bool as true (high) or false (low)
    */
    bool read();
    
    /**
    * Overloads bool operation, returns read()
    *
    * @return bool as true (high) or false (low)
    */
    operator bool() {
        return read();
    }

protected:
    PinName pin;
};
