#pragma once

#include "Assert.hpp"
#include "Mbed.hpp"
#include "Rtos.hpp"

#include <memory>

/**
 * A simple wrapper over mbed's SPI class that includes a mutex.
 *
 * This makes it easier to correctly use a shared SPI bus (with multiple
 * devices) in a multi-threaded environment.
 */
class SharedSPI : public mbed::SPI, public Mutex {
public:
    SharedSPI(PinName mosi, PinName miso, PinName sck) : SPI(mosi, miso, sck) {
        ++m_objectCount;
        ASSERT(m_objectCount == 1);
    }

private:
    static uint8_t m_objectCount;
};

/**
 * Classes that provide an interface to a device on the shared spi bus should
 * inherit from this class.
 */
template <class DIGITAL_OUT = mbed::DigitalOut>
class SharedSPIDevice {
public:
    using SpiPtrT = std::shared_ptr<SharedSPI>;

    SharedSPIDevice(SpiPtrT spi, DIGITAL_OUT cs, bool csInverted = true)
        : m_spi(spi), m_cs(cs) {
        ASSERT(spi != nullptr);

        // The value we set the chip select pin to in order to assert it (it's
        // often inverted).
        m_csAssertValue = csInverted ? 0 : 1;

        // Initialize to a de-asserted state
        m_cs = !m_csAssertValue;
    }

    void chipSelect() {
        m_spi->lock();
        m_spi->frequency(m_frequency);
        m_cs = m_csAssertValue;
    }

    void chipDeselect() {
        m_cs = !m_csAssertValue;
        m_spi->unlock();
    }

    /// Set the SPI frequency for this device
    void setSPIFrequency(int hz) { m_frequency = hz; }

    SpiPtrT m_spi;
protected:
    DIGITAL_OUT m_cs;

private:
    int m_csAssertValue;

    /// The SPI bus frequency used by this device.
    /// This default value is the same as the mbed's default (1MHz).
    int m_frequency = 1'000'000;
};
