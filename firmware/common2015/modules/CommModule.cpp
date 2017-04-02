#include "CommModule.hpp"

#include "Assert.hpp"
#include "CommPort.hpp"
#include "Console.hpp"
#include "HelperFuncs.hpp"
#include "Logger.hpp"

#include <ctime>

std::shared_ptr<CommModule> CommModule::Instance = nullptr;

CommModule::CommModule(std::shared_ptr<FlashingTimeoutLED> rxTimeoutLED,
                       std::shared_ptr<FlashingTimeoutLED> txTimeoutLED)
    : m_rxThread(&CommModule::rxThreadHelper, this, RX_PRIORITY, STACK_SIZE),
      m_txThread(&CommModule::txThreadHelper, this, TX_PRIORITY, STACK_SIZE),
      m_rxTimeoutLED(rxTimeoutLED),
      m_txTimeoutLED(txTimeoutLED) {
    // Create the data queues.
    m_txQueue = osMailCreate(m_txQueueHelper.def(), nullptr);
    m_rxQueue = osMailCreate(m_rxQueueHelper.def(), nullptr);
}

void CommModule::txThread() {
    // Only continue once we know there's 1+ hardware link(s) available
    Thread::signal_wait(SIGNAL_START);

    // Store our priority so we know what to reset it to if ever needed
    const auto threadPriority = m_txThread.get_priority();

    LOG(OK, "TX communication module ready!\r\n    Thread ID: %u, Priority: %d",
        reinterpret_cast<P_TCB>(m_rxThread.gettid())->task_id, threadPriority);

    // Signal to the RX thread that it can begin
    m_rxThread.signal_set(SIGNAL_START);

    while (true) {
        // Wait until new data is placed in the RX queue
        auto evt = osMailGet(m_txQueue, osWaitForever);

        if (evt.status == osEventMail) {
            // Get a pointer to the packet's memory location
            auto p = static_cast<RTP::Packet*>(evt.value.p);

// Bump up the thread's priority
#ifndef NDEBUG
            auto tState = m_txThread.set_priority(osPriorityRealtime);
            ASSERT(tState == osOK);
#else
            m_txThread.set_priority(osPriorityRealtime);
#endif

            // cache the dereference to the header
            const auto& header = p->header;

            // this renews a countdown for turning off the strobing thread
            if (header.address != RTP::LOOPBACK_ADDRESS && m_txTimeoutLED) {
                m_txTimeoutLED->renew();
            }

            // grab the port number
            const auto portNum = header.port;

            // grab an iterator to the port, lookup only once
            const auto portIter = m_ports.find(portNum);

            // invoke callback if port exists and has an attached function
            if (portIter != m_ports.end()) {
                if (portIter->second.hasTxCallback()) {
                    portIter->second.getTxCallback()(p);

                    LOG(INFO,
                        "Transmission:\r\n"
                        "    Port:\t%u\r\n",
                        portNum);
                }
            }

            // Release the allocated memory once data is sent
            osMailFree(m_txQueue, p);

#ifndef NDEBUG
            tState = m_txThread.set_priority(threadPriority);
            ASSERT(tState == osOK);
#else
            m_txThread.set_priority(threadPriority);
#endif
        }
    }

    ASSERT(!"Execution is at an unreachable line!");
}

void CommModule::rxThread() {
    // Only continue once we know there's 1+ hardware link(s) available
    Thread::signal_wait(SIGNAL_START);

    // set this true immediately after we are released execution
    m_isRunning = true;

    // Store our priority so we know what to reset it to if ever needed
    const auto threadPriority = m_rxThread.get_priority();

    LOG(OK, "RX communication module ready!\r\n    Thread ID: %u, Priority: %d",
        reinterpret_cast<P_TCB>(m_rxThread.gettid())->task_id, threadPriority);

    while (true) {
        // Wait until new data is placed in the RX queue
        auto evt = osMailGet(m_rxQueue, osWaitForever);

        if (evt.status == osEventMail) {
            // get a pointer to where the data is stored
            auto p = static_cast<RTP::Packet*>(evt.value.p);

// Bump up the thread's priority
#ifndef NDEBUG
            auto tState = m_rxThread.set_priority(osPriorityRealtime);
            ASSERT(tState == osOK);
#else
            m_rxThread.set_priority(osPriorityRealtime);
#endif

            // cache the dereference to the header
            const auto& header = p->header;

            // this renews a countdown for turning off the strobing thread
            if (header.address != RTP::LOOPBACK_ADDRESS && m_rxTimeoutLED) {
                m_rxTimeoutLED->renew();
            }

            // grab the port number
            const auto portNum = header.port;

            // grab an iterator to the port, lookup only once
            const auto portIter = m_ports.find(portNum);

            // invoke callback if port exists and has an attached function
            if (portIter != m_ports.end()) {
                if (portIter->second.hasRxCallback()) {
                    portIter->second.getRxCallback()(std::move(*p));

                    LOG(INFO,
                        "Reception:\r\n"
                        "    Port:\t%u\r\n",
                        portNum);
                }
            }

            // free memory allocated for mail
            osMailFree(m_rxQueue, p);

#ifndef NDEBUG
            tState = m_rxThread.set_priority(threadPriority);
            ASSERT(tState == osOK);
#else
            m_rxThread.set_priority(threadPriority);
#endif
        }
    }

    ASSERT(!"Execution is at an unreachable line!");
}

void CommModule::send(RTP::Packet packet) {
    const auto portNum = packet.header.port;
    const auto portExists = m_ports.find(portNum) != m_ports.end();
    const auto hasCallback = m_ports[portNum].hasTxCallback();

    // Check to make sure a socket for the port exists
    if (portExists && hasCallback) {
        // Allocate a block of memory for the data.
        auto p =
            static_cast<RTP::Packet*>(osMailAlloc(m_txQueue, osWaitForever));
        if (p) {
            // Copy the contents into the allocated memory block
            *p = std::move(packet);
            // Place the passed packet into the txQueue.
            osMailPut(m_txQueue, p);
        } else {
            LOG(SEVERE, "Unable to allocate memory for TX queue");
        }
    } else {
        LOG(WARN,
            "Failed to send %u byte packet: No TX socket on port %u exists",
            packet.payload.size(), packet.header.port);
    }
}

void CommModule::receive(RTP::Packet packet) {
    const auto portNum = packet.header.port;
    const auto portExists = m_ports.find(portNum) != m_ports.end();
    const auto hasCallback = m_ports[portNum].hasRxCallback();

    // Check to make sure a socket for the port exists
    if (portExists && hasCallback) {
        // Allocate a block of memory for the data.
        auto p =
            static_cast<RTP::Packet*>(osMailAlloc(m_rxQueue, osWaitForever));
        if (p) {
            // Move the contents into the allocated memory block
            *p = std::move(packet);
            // Place the passed packet into the rxQueue.
            osMailPut(m_rxQueue, p);
        } else {
            LOG(SEVERE, "Unable to allocate memory for RX queue");
        }
    } else {
        LOG(WARN,
            "Failed to send %u byte packet: No RX socket on port %u exists",
            packet.payload.size(), packet.header.port);
    }
}

void CommModule::setRxHandler(RxCallbackT callback, uint8_t portNbr) {
    m_ports[portNbr].setRxCallback(std::bind(callback, std::placeholders::_1));
    ready();
}

void CommModule::setTxHandler(TxCallbackT callback, uint8_t portNbr) {
    m_ports[portNbr].setTxCallback(std::bind(callback, std::placeholders::_1));
    ready();
}

void CommModule::ready() {
    if (m_isReady) m_txThread.signal_set(SIGNAL_START);
    m_isReady = true;
}

void CommModule::close(unsigned int portNbr) noexcept {
    try {
        m_ports.erase(portNbr);
    } catch (...) {
    }
}

#ifndef NDEBUG
unsigned int CommModule::numOpenSockets() const {
    auto count = 0;
    for (const auto& kvpair : m_ports) {
        if (kvpair.second.hasRxCallback() || kvpair.second.hasTxCallback())
            ++count;
    }
    return count;
}

unsigned int CommModule::numRxPackets() const {
    auto count = 0;
    for (const auto& kvpair : m_ports) count += kvpair.second.getRxCount();
    return count;
}

unsigned int CommModule::numTxPackets() const {
    auto count = 0;
    for (const auto& kvpair : m_ports) count += kvpair.second.getTxCount();
    return count;
}

void CommModule::resetCount(unsigned int portNbr) {
    m_ports[portNbr].resetCounts();
}

void CommModule::printInfo() const {
    printf("PORT\t\tIN\tOUT\tRX CBCK\t\tTX CBCK\r\n");

    for (const auto& kvpair : m_ports) {
        const PortT& p = kvpair.second;
        printf("%d\t\t%u\t%u\t%s\t\t%s\r\n", kvpair.first, p.getRxCount(),
               p.getTxCount(), p.hasRxCallback() ? "YES" : "NO",
               p.hasTxCallback() ? "YES" : "NO");
    }

    printf(
        "==========================\r\n"
        "Total:\t\t%u\t%u\r\n",
        numRxPackets(), numTxPackets());

    Console::Instance->Flush();
}
#endif
