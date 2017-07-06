#pragma once

#include "Mbed.hpp"
#include "Rtos.hpp"

#include "CommPort.hpp"
#include "HelperFuncs.hpp"
#include "RTP.hpp"
#include "TimeoutLED.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <deque>

/**
 * @brief A high-level firmware class for packet handling & routing
 *
 * The CommModule class provides a software routing service
 * by filtering and routing RX packets to a respective callback
 * function and sending TX packets to a derived CommLink class.
 */
class CommModule {
public:
    /// Type aliases
    using RxCallbackSigT = void(RTP::Packet);
    using TxCallbackSigT = uint32_t(const RTP::Packet*);
    using RxCallbackT = std::function<RxCallbackSigT>;
    using TxCallbackT = std::function<TxCallbackSigT>;
    using PortT = CommPort<RxCallbackSigT, TxCallbackSigT>;

    /// Global singleton instance of CommModule
    static std::shared_ptr<CommModule> Instance;

    /// The constructor initializes and starts threads and mail queues
    CommModule();

    /// Assign an RX callback function to a port
    void setRxHandler(RxCallbackT callback, uint8_t portNbr);

    /// Assign a TX callback function to a port
    void setTxHandler(TxCallbackT callback, uint8_t portNbr);

    /// Assign an RX callback method to a port
    template <typename B>
    void setRxHandler(B* obj, void (B::*mptr)(RTP::Packet), uint8_t portNbr) {
        setRxHandler(std::bind(mptr, obj, std::placeholders::_1), portNbr);
    }

    /// Assign an TX callback method to a port
    template <typename B>
    void setTxHandler(B* obj, int32_t (B::*mptr)(const RTP::Packet*),
                      uint8_t portNbr) {
        setTxHandler(std::bind(mptr, obj, std::placeholders::_1), portNbr);
    }

    /// Send an RTP::Packet over previsouly initialized hardware
    void send(RTP::Packet pkt);

    /// Called by CommLink instances whenever a packet is received via radio
    void receive(RTP::Packet pkt);

    /// Close a port that was previouly assigned callback functions/methods
    void close(unsigned int portNbr) noexcept;

    /// Check if everything is ready for sending/receiving packets
    inline bool isReady() const noexcept { return m_isReady && m_isRunning; }

#ifndef NDEBUG
    /// Retuns the number of ports with a binded callback function/method
    unsigned int numOpenSockets() const;

    /// Retuns the number of currently received packets
    unsigned int numRxPackets() const;

    /// Retuns the number of currently sent packets
    unsigned int numTxPackets() const;

    /// Resets the counts for send/received packets
    void resetCount(unsigned int portNbr);

    /// Print debugging information
    void printInfo() const;
#endif

protected:
    static constexpr size_t SIGNAL_START = (1 << 0);

    osPoolId m_txPoolId;
    osPoolId m_rxPoolId;

    osMessageQId m_txMessageQueue;
    osMessageQId m_rxMessageQueue;

private:
    // DEFAULT_STACK_SIZE defined in rtos library
    static constexpr size_t STACK_SIZE = DEFAULT_STACK_SIZE / 2;

    std::map<uint8_t, PortT> m_ports;

    Thread m_rxThread;
    Thread m_txThread;

    osThreadId m_rxThreadId;
    osThreadId m_txThreadId;

    bool m_isReady{ false };
    bool m_isRunning{ false };

    void ready();
    void txThread();
    void rxThread();

    // The threadHelper methods accept a CommModule pointer as a parameter
    inline static void rxThreadHelper(const void* moduleInst) {
        auto module = reinterpret_cast<CommModule*>(
            const_cast<void*>(moduleInst));  // dangerous
        module->rxThread();
    }
    inline static void txThreadHelper(const void* moduleInst) {
        auto module = reinterpret_cast<CommModule*>(
            const_cast<void*>(moduleInst));  // dangerous
        module->txThread();
    }
};
