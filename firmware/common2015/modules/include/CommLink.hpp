#pragma once

#include "CommModule.hpp"
#include "MacroHelpers.hpp"
#include "MailHelpers.hpp"
#include "RTP.hpp"
#include "Rtos.hpp"
#include "SharedSPI.hpp"

#include <memory>

/// CommLink error levels
enum { COMM_SUCCESS, COMM_FAILURE, COMM_DEV_BUF_ERR, COMM_NO_DATA };

/**
 * CommLink Class used as the hal (hardware abstraction layer) module for
 * interfacing communication links to the higher-level firmware
 */
class CommLink : public SharedSPIDevice<> {
public:
    // Type aliases
    using BufferT = std::vector<uint8_t>;
    using BufferPtrT = BufferT*;
    using ConstBufferPtrT = const BufferT*;

    // Class constants for data queues
    static constexpr size_t RX_QUEUE_SIZE = 2;

    /// Constructor
    CommLink(SpiPtrT spiBus, PinName nCs = NC, PinName intPin = NC);

    /// Kills any threads and frees the allocated stack.
    virtual ~CommLink() = default;

    // CommLink(const CommLink&) = default;
    // CommLink& operator=(const CommLink&) = default;
    //
    // CommLink(CommLink&&) = default;
    // CommLink& operator=(CommLink&&) = default;

    // The pure virtual methods for making CommLink an abstract class
    /// Perform a soft reset for a communication link's hardware device
    virtual void reset() = 0;

    /// Perform tests to determine if the hardware is able to properly function
    virtual int32_t selfTest() = 0;

    /// Determine if communication can occur with another device
    virtual bool isConnected() const = 0;

    /// Send & Receive through the rtp structure
    virtual int32_t sendPacket(const RTP::Packet* pkt) = 0;

    /// Set the MAC layer filtering address for the link
    virtual void setAddress(int addr) { m_address = addr; }

protected:
    static constexpr size_t SIGNAL_START = (1 << 1);
    static constexpr size_t SIGNAL_RX = (1 << 1);

    int m_address = RTP::INVALID_ROBOT_UID;
    InterruptIn m_intIn;

    /**
     * @brief Read data from the radio's RX buffer
     *
     * @param buf The buffer to write data into
     *
     * @return A vector of received bytes returned with std::move
     */
    virtual BufferT getData() = 0;

    /// Interrupt Service Routine
    void ISR() { m_rxThread.signal_set(SIGNAL_RX); }

    /// Called by the derived class to begin thread operations
    void ready() { m_rxThread.signal_set(SIGNAL_START); }

    /// Bump up the priority of the RX thread
    osStatus raiseThreadPriority() {
        return m_rxThread.set_priority(osPriorityHigh);
    };

    template <typename T>
    constexpr static T twos_compliment(T val) {
        return ~val + 1;
    }

private:
    Thread m_rxThread;

    // The working thread for handling RX data queue operations
    void rxThread();

    inline static void rxThreadHelper(const void* linkInst) {
        auto link = reinterpret_cast<CommLink*>(
            const_cast<void*>(linkInst));  // dangerous
        link->rxThread();
    }
};

extern std::unique_ptr<CommLink> globalRadio;
