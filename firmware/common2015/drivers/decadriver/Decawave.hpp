#pragma once

#include "CommLink.hpp"
#include "mbed.h"
#include "rtos.h"

#include "deca_device_api.hpp"
#include "deca_regs.h"

#define FRAME_LEN_MAX 127

class Decawave : public CommLink, public dw1000_api {
public:
    Decawave(std::shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName intPin);

    int32_t sendPacket(const rtp::packet* pkt);
    int32_t getData(std::vector<uint8_t>* buf);
    void reset();
    int32_t selfTest();
    bool isConnected() const;

    int writetospi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 bodylength, const uint8 *bodyBuffer);
    int readfromspi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 readlength, uint8 *readBuffer);

    decaIrqStatus_t decamutexon(void);
    void decamutexoff(decaIrqStatus_t s);
    void deca_sleep(unsigned int time_ms);

    void printStuff();

    void setAddress(uint16_t addr);
    void logSPI(int num);
    void setLED(bool ledOn);
private:
    uint32_t _chip_version;
    uint8 rx_buffer[100]; //TODO: better tx and rx buffer
    uint8 tx_buffer[100];
    bool _isInit;

    uint32_t rx_status;
    uint8_t rx_len;
    uint8_t _addr = rtp::INVALID_ROBOT_UID;

    void getData_success(const dwt_cb_data_t *cb_data);
    void getData_fail(const dwt_cb_data_t *cb_data);

};

// int readfromspi(uint16 headerLength, const uint8 *headerBuffer,
//                         uint32 readlength, uint8 *readBuffer) {
//     LOG(INIT, "spi 1");
//     return global_radio->readfromspi(headerLength, headerBuffer, readlength, readBuffer);
// }
//
// int writetospi(uint16 headerLength, const uint8 *headerBuffer,
//                         uint32 bodylength, const uint8 *bodyBuffer) {
//     return global_radio->writetospi(headerLength, headerBuffer, bodylength, bodyBuffer);
// }

extern Decawave* global_radio;
// Decawave& global_radio() {static Decawave radio; return radio}