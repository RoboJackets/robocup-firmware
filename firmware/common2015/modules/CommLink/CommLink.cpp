#include "CommLink.hpp"
#include "Decawave.hpp"

#include "assert.hpp"
#include "logger.hpp"

#define COMM_LINK_SIGNAL_START_THREAD (1 << 0)
#define COMM_LINK_SIGNAL_RX_TRIGGER (1 << 1)

const char* COMM_ERR_STRING[] = {FOREACH_COMM_ERR(GENERATE_STRING)};

CommLink::CommLink(shared_ptr<SharedSPI> sharedSPI, PinName nCs,
                   PinName int_pin)
    : SharedSPIDevice(sharedSPI, nCs, true),
      _int_in(int_pin),
      _rxThread(&CommLink::rxThreadHelper, this, osPriorityNormal,
                DEFAULT_STACK_SIZE / 2) {
    setSPIFrequency(5000000);
    _int_in.mode(PullDown);
}

// =================== RX THREAD ===================
// Task operations for placing received data into the received data queue
void CommLink::rxThread() {
    // Store our priority so we know what to reset it to if ever needed
    const osPriority threadPriority = _rxThread.get_priority();

    // Set the function to call on an interrupt trigger
    _int_in.rise(this, &CommLink::ISR);

    std::vector<uint8_t> buf;
    buf.reserve(rtp::MAX_DATA_SZ);
    printf("before");
    // Only continue past this point once the hardware link is initialized
    Thread::signal_wait(COMM_LINK_SIGNAL_START_THREAD);
    printf("after");
    LOG(INIT, "RX communication link ready!\r\n    Thread ID: %u, Priority: %d",
        ((P_TCB)_rxThread.gettid())->task_id, threadPriority);

    // dwt_setrxaftertxdelay(60);
    // dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    // dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    // dwt_write32bitreg(SYS_STATUS_ID,SYS_STATUS_ALL_TX);
    // dwt_rxenable(DWT_START_RX_IMMEDIATE);
    printf("after22");
    while (true) {
        // Wait until new data has arrived
        // this is triggered by CommLink::ISR()
        // if (((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))) {
        //     printf("test\r\n");
        // }
        // printf("%p", (dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));
        // static uint32 status_reg = 0;
        // while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))) {};
        // printf("%p", status_reg);

        // dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        // dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // printf("before\r\n");
        // printf("INTERRUPT: %d\r\n", _int_in.read());
        Thread::signal_wait(COMM_LINK_SIGNAL_RX_TRIGGER);

        LOG(INF3, "RX interrupt triggered");

        // Get the received data from the external chip
        buf.clear();
        int32_t response = getData(&buf);
        // dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFFUL);
        // dwt_rxenable(DWT_START_RX_IMMEDIATE);

        if (response == COMM_SUCCESS) {
            // Write the data to the CommModule object's rxQueue
            printf("got something\r\n");
            rtp::packet p;
            p.recv(buf);
            CommModule::Instance->receive(std::move(p));
        }
    }
}

// Called by the derived class to begin thread operations
void CommLink::ready() { _rxThread.signal_set(COMM_LINK_SIGNAL_START_THREAD); }

void CommLink::ISR() { _rxThread.signal_set(COMM_LINK_SIGNAL_RX_TRIGGER); }
