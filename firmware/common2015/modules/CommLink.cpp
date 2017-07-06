#include "CommLink.hpp"

#include "Assert.hpp"
#include "Logger.hpp"

namespace {
// DEFAULT_STACK_SIZE defined in rtos library
constexpr auto STACK_SIZE = DEFAULT_STACK_SIZE / 2;
constexpr auto RX_PRIORITY = osPriorityHigh;
}

std::unique_ptr<CommLink> globalRadio = nullptr;

CommLink::CommLink(SpiPtrT sharedSPI, PinName nCs, PinName intPin)
    : SharedSPIDevice(sharedSPI, nCs, true),
      m_intIn(intPin),
      m_rxThread(&CommLink::rxThreadHelper, this, RX_PRIORITY, STACK_SIZE) {
    setSPIFrequency(5'000'000);
    m_intIn.mode(PullDown);
    ready();
}

// Task operations for placing received data into the received data queue
void CommLink::rxThread() {
    // Store our priority so we know what to reset it to if ever needed
    const auto threadPriority = m_rxThread.get_priority();
    (void)threadPriority;  // disable compiler warning for unused-variable
    ASSERT(threadPriority != osPriorityError);

    // Set the function to call on an interrupt trigger
    m_intIn.rise(this, &CommLink::ISR);

    // Only continue past this point once the hardware link is initialized
    Thread::signal_wait(SIGNAL_START);

    LOG(OK, "RX communication link ready!\r\n    Thread ID: %u, Priority: %d",
        reinterpret_cast<P_TCB>(m_rxThread.gettid())->task_id, threadPriority);

    while (true) {
        // Wait until new data has arrived
        Thread::signal_wait(SIGNAL_RX);

        LOG(DEBUG, "RX interrupt triggered");

        // Get the received data from the external chip
        auto response = getData();

        if (!response.empty()) {
            // Write the data to the CommModule object's rxQueue
            CommModule::Instance->receive(RTP::Packet(response));
        }
    }

    ASSERT(!"Execution is at an unreachable line!");
}
