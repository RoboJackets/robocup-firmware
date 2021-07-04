#pragma once

#include <array>

/**
 * This class facilitates reading the value from a rotary selector.
 *
 * This class is templated so that it generalizes I/O functionality to not only
 * IO expander pins but also standard mTrain I/O pins.
 *
 * Example usage:
 * RotarySelector<DigitalIn> selector({p5, p6, p7, p8});
 * printf("selector value: %d\r\n", selector.read());
 */
template <typename DIGITAL_IN, size_t NUM_PINS = 4>
class RotarySelector {
public:
    /**
     * Initialize a RotarySelector with the given digital input pins
     *
     * @param pins An array of pins ordered from least significant to most
     */
    RotarySelector(std::array<DIGITAL_IN, NUM_PINS> pins) : m_pins(pins) {}

    void init() {
        for (auto& pin : m_pins) {
            pin.init();
        }
    }

    /**
     * Gives the reading (0x0 - 0xF) of the NUM_PINS wires
    */
    uint8_t read() {
        uint8_t reading = 0;
        for (size_t i = 0; i < NUM_PINS; ++i) reading |= m_pins[i].read() << i;
        return reading;
        // return 1;
    }

private:
    std::array<DIGITAL_IN, NUM_PINS> m_pins;
};
