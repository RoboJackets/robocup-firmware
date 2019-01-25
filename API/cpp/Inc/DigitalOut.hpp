#pragma once

#include "PinDefs.hpp"

typedef enum {
    PUSH_PULL = GPIO_MODE_OUTPUT_PP,
    OPEN_DRAIN = GPIO_MODE_OUTPUT_OD
} PinMode;

typedef enum {
    LOW = GPIO_SPEED_FREQ_LOW,
    MEDIUM = GPIO_SPEED_FREQ_MEDIUM,
    HIGH = GPIO_SPEED_FREQ_HIGH,
    VERY_HIGH = GPIO_SPEED_FREQ_VERY_HIGH
} PinSpeed;

class DigitalOut {
public:
    /**
    * Initializes Digital Out
    *
    * @param PinName pin
    * @default PullType PULL_NONE, PinMode PUSH_PULL, PinSpeed LOW, bool false
    */
    DigitalOut(PinName pin, PullType pull = PullType::PullNone,
        PinMode mode = PinMode::PUSH_PULL, PinSpeed speed = PinSpeed::LOW,
        bool inverted = false);
    
    /**
    * Deinitalizes Digital Out
    */
    ~DigitalOut();

    /**
    * Writes pin state
    *
    * @param bool state as true (high) or false (low), opposite if inverted
    */
    void write(bool state);
    /**
    * Toggles pin state
    */
    void toggle();
    
    /**
    * Returns pin state
    *
    * @return bool as true (high) or false (low)
    */
    bool read();
    
    /**
    * Overloads assignment operation, runs write()
    */
    void operator =(bool rhs) {
        write(rhs);
    }
    /**
    * Overloads bool operation, returns read()
    *
    * @return bool pin state as true (high) or false (low)
    */
    operator bool() {
        return read();
    }

protected:
    PinName pin;
    bool inverted;
};
