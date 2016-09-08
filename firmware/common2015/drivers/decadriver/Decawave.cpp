#include "Decawave.hpp"

//#include "assert.hpp"
//#include "logger.hpp"

Decawave* global_radio_2 = nullptr;

static dwt_config_t config = {
    1,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_1024,   /* Preamble length. Used in TX only. */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_110K,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

static dwt_txconfig_t txconfig = {
    0xC9,            /* PG delay. */
    0x67676767,      /* TX power. */
};

#define TX_TO_RX_DELAY_UUS 60
#define RX_RESP_TO_UUS 5000

Decawave::Decawave(shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName intPin)
    : CommLink(sharedSPI, nCs, intPin) {

    //TODO: Don't forget to fix SPI

    if (dwt_initialise(DWT_LOADNONE) == DWT_ERROR) {
        LOG(FATAL,"Decawave not initialized");
        _isInit = false;
        return;
    }

    reset();
    selfTest();

    if (_isInit) {
        dwt_configure(&config);
        dwt_configuretxrf(&txconfig);

        dwt_setrxaftertxdelay(TX_TO_RX_DELAY_UUS);
        dwt_setrxtimeout(RX_RESP_TO_UUS);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        CommLink::ready();
    }
}

int32_t Decawave::sendPacket(const rtp::packet* pkt){
    // Return failutre if not initialized
    if (!_isInit) return COMM_FAILURE;

    // Copy header and payload of packet to buffer
    size_t i;
    for (i = 0; i < sizeof(pkt->header); ++i) {
        tx_buffer[i] = ((uint8_t*)&pkt->header)[i];
    }
    for (uint8_t byte : pkt->payload) {
        i++;
        tx_buffer[i] = byte;
    }

    dwt_writetxdata(sizeof(tx_buffer), tx_buffer, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(tx_buffer), 0, 0); /* Zero offset in TX buffer, no ranging. */
    if (DWT_SUCCESS == dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED)) return COMM_SUCCESS;

    return COMM_DEV_BUF_ERR;
}

int32_t Decawave::getData(std::vector<uint8_t>* buf){
    // Return failutre if not initialized
    if (!_isInit) return COMM_FAILURE;

    uint32 status_reg = dwt_read32bitreg(SYS_STATUS_ID);

    if (status_reg & SYS_STATUS_RXFCG) { //TODO: correct status check?
        // Radio recieved FCS good

        // Read recieved frame length from register
        uint16 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
        if (frame_len > FRAME_LEN_MAX) {
            LOG(WARN, "Frame recieved too large:\r\n"
                "   Recieved: %u Max: %u", FRAME_LEN_MAX, frame_len);
            return COMM_DEV_BUF_ERR;
        }

        // Read recived data and copy to vector
        dwt_readrxdata(rx_buffer, frame_len, 0);
        for (uint8_t i = 0; i < frame_len; i++) {
            buf->push_back(rx_buffer[i]); // TODO: in reverse?
        }

        // Clear good RX frame event in the status register
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

        return COMM_SUCCESS;
    } else if (status_reg & (SYS_STATUS_ALL_RX_TO| SYS_STATUS_ALL_RX_ERR)) {
        // Radio RX error or timeout

        // Clear RX error/timeout events in the status register
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        return COMM_DEV_BUF_ERR;
    }

    return COMM_NO_DATA;
}

void Decawave::reset(){
    dwt_softreset();
}

int32_t Decawave::selfTest(){
    _chip_version = dwt_readdevid();
    if (_chip_version != DWT_DEVICE_ID) {
        LOG(FATAL,
            "Decawave part number error:\r\n"
            "    Found:\t0x%02X (expected 0x%02X)",
            _chip_version, DWT_DEVICE_ID);

        return -1;
    } else {
        return 0;
    }
}

bool Decawave::isConnected() const {
    return _isInit;
}

void Decawave::setLED(bool ledOn) {
    dwt_setleds(ledOn);
}