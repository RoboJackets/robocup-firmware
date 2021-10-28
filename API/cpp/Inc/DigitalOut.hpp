#pragma once

#include "PinDefs.hpp"

typedef enum {
    PushPull = GPIO_MODE_OUTPUT_PP,
    OpenDrain = GPIO_MODE_OUTPUT_OD
} PinMode;

typedef enum {
    Low = GPIO_SPEED_FREQ_LOW,
    Medium = GPIO_SPEED_FREQ_MEDIUM,
    High = GPIO_SPEED_FREQ_HIGH,
    VeryHigh = GPIO_SPEED_FREQ_VERY_HIGH
} PinSpeed;

class DigitalOut {
public:
    /** Configures GPIO pin for digital in
     *
     * @param pin Pin def external to board
     * @param pull Pin pull type
     * @param mode Output mode (push-pull or open-drain)
     * @param speed GPIO frequency
     * @param inverted Inverts output value of pin
     */
    DigitalOut(PinName pin, PullType pull = PullType::PullNone,
        PinMode mode = PinMode::PushPull, PinSpeed speed = PinSpeed::Low,
        bool inverted = false);
    
    ~DigitalOut();

    /** Change output state of pin
     *
     * @param state Set output value of pin (0 or 1)
     *     In open-drain 0 is high-z, 1 is drive low
     */
    void write(bool state);

    /** Toggle pin output
     * 
     */
    void toggle();
    
    /** Read current value of pin
     *
     * @return true (high) or false (low)
     */
    bool read();
    
    void operator =(bool rhs) {
        write(rhs);
    }

    operator bool() {
        return read();
    }

protected:
    PinName pin;
    bool inverted;
};
