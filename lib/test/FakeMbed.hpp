#pragma once

#include <cassert>
#include <vector>

/**
 * This file provides some helper classes for emulating an mbed's hardware.  It
 * is useful for unit-testing code on a computer when an mbed is not available
 * or convenient.
 */
namespace MbedTest {

class DigitalIn {
public:
    DigitalIn(int value) : m_value(value) {}

    int read() const { return m_value; }
    operator int() { return read(); }

private:
    int m_value;
};

const auto NC = 0xFFFFFFFF;

class SPI {
public:
    int m_mosi = NC;
    int m_miso = NC;
    int m_sclk = NC;
    int m_ssel = NC;
    std::vector<int> m_rxData;

    SPI(int mosi, int miso, int sclk, int ssel = NC)
        : m_mosi(mosi), m_miso(miso), m_sclk(sclk), m_ssel(ssel) {}

    void format(int bits, int mode = 0) {
        m_bits = bits;
        m_mode = mode;
    }

    void frequency(int hz = 1'000'000) { m_hz = hz; }

    int write(int value) {
        assert(!m_rxData.empty());

        const auto rxByte = m_rxData.at(0);
        m_rxData.erase(m_rxData.begin());
        return rxByte;
    }

private:
    int m_bits;
    int m_mode;
    int m_hz;
};

}  // namespace MbedTest
