#pragma once

#include "CommLink.hpp"
#include "mbed.h"
#include "rtos.h"

#include "deca_device_api.h"
#include "deca_regs.h"

#define FRAME_LEN_MAX 127

class Decawave : public CommLink {
public:
    Decawave(std::shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName intPin);

    int32_t sendPacket(const rtp::packet* pkt);
    int32_t getData(std::vector<uint8_t>* buf);

    void reset();
    int32_t selfTest();
    bool isConnected() const;

    void setLED(bool ledOn);
private:
    uint32_t _chip_version;
    static uint8 rx_buffer[FRAME_LEN_MAX];
    static uint8 tx_buffer[FRAME_LEN_MAX];
    bool _isInit;
};

extern Decawave* global_radio_2;