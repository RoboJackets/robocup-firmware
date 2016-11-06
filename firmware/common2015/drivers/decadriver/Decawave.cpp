// #include "deca_device_api.h"
#include "Decawave.hpp"

#include "assert.hpp"
#include "logger.hpp"

Decawave* global_radio = nullptr;

static dwt_config_t config = {
    4,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,   /* Preamble length. Used in TX only. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    17,               /* TX preamble code. Used in TX only. */
    17,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (128 + 1 + 64 - 8) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

static dwt_txconfig_t txconfig = {
    0x95,            /* PG delay. */
    0x9A9A9A9A,      /* TX power. */
};

#define TX_TO_RX_DELAY_UUS 60
#define RX_RESP_TO_UUS 5000

Decawave::Decawave(shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName intPin)
    : CommLink(sharedSPI, nCs, intPin), dw1000_api() {
    // dw1000_api();
    global_radio = this; // TODO: This is very not good
    setSPIFrequency(2000000);
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
        //dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RPHE | DWT_INT_RFCG | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_RFTO | DWT_INT_SFDT | DWT_INT_RXPTO | DWT_INT_ARFE, 1);
        dwt_setcallbacks(NULL, static_cast<dwt_cb_t>(&Decawave::getData_success), NULL, static_cast<dwt_cb_t>(&Decawave::getData_fail));
        // dwt_setinterrupt(DWT_INT_RFCG | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);
        dwt_setinterrupt(0x00002000 | DWT_INT_RFCG | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_SFDT | DWT_INT_ARFE, 1);
        // dwt_setinterrupt(DWT_INT_RFCG, 1);

        dwt_setautorxreenable(1);

        setLED(true);
        dwt_forcetrxoff(); // TODO: Better way than force off then reset?
        dwt_rxreset();
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        setSPIFrequency(10000000); // 10 MHz

        LOG(INIT, "Decawave ready!");
        CommLink::ready();
    }
}

// Virtual functions from CommLink

int32_t Decawave::sendPacket(const rtp::packet* pkt){
    // Return failutre if not initialized
    // LOG(INIT,"SENDING");
    if (!_isInit) return COMM_FAILURE;
    dwt_rxreset();
    dwt_forcetrxoff();

    // Copy header and payload of packet to buffer

    //0x8841
    tx_buffer[0] = 0x41;
    tx_buffer[1] = 0x88;
    tx_buffer[2] = 0;
    tx_buffer[3] = 0xCA;
    tx_buffer[4] = 0xDE;
    tx_buffer[5] = pkt->header.address;
    tx_buffer[6] = 0;
    tx_buffer[7] = _addr;
    tx_buffer[8] = 0;


    /*
    tx_buffer[0] = 0xC5;
    tx_buffer[1] = 0;
    */

    uint8_t i;
    uint8_t* headerData = (uint8_t*)&pkt->header;
    for (i = 0; i < sizeof(pkt->header); ++i) {
        tx_buffer[i+9] = headerData[i];
        // printf("%d ",tx_buffer[i]);
    }
    i+=8;
    for (uint8_t byte : pkt->payload) {
        i++;
        tx_buffer[i] = byte;
        // printf("%d ",tx_buffer[i]);
    }
    // printf("\r\n");
    tx_buffer[i + 1] = 0;
    tx_buffer[i + 2] = 0;

    dwt_writetxdata(i+3, tx_buffer, 0);
    dwt_writetxfctrl(i+3, 0, 0);

    if (DWT_SUCCESS == dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED)) {
        // LOG(INIT, "0x%02X", dwt_read32bitreg(SYS_CFG_ID));
        return COMM_SUCCESS;
    }

    return COMM_DEV_BUF_ERR;
}

void Decawave::printStuff() {
    // LOG(INIT, "0x%02X", dwt_read32bitreg(SYS_CFG_ID));
    // LOG(INIT, "0x%02X", dwt_read32bitreg(SYS_STATUS_ID));
}

int32_t Decawave::getData(std::vector<uint8_t>* buf){
    // Return failutre if not initialized
    // LOG(INIT, "Getting data");
    if (!_isInit) return COMM_FAILURE;

    rx_status = COMM_NO_DATA;
    rx_len = 0; // Probably not needed

    dwt_isr();
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    // LOG(INIT, "Something %u, want %u, not %u, len %u", rx_status, COMM_SUCCESS, COMM_DEV_BUF_ERR, rx_len);
    if (rx_status == COMM_SUCCESS) {
        // LOG(INIT, "SUCCESS?");
        for (uint8_t i = 9; i < rx_len - 2; i++) {
            // printf("%d ",rx_buffer[i]);
            buf->push_back(rx_buffer[i]);
        }
        // printf("\r\n");
    }

    return rx_status;
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
        _isInit = true;
        return 0;
    }
}

bool Decawave::isConnected() const {
    return _isInit;
}

void Decawave::setAddress(uint16_t addr) {
    _addr = addr;
    dwt_setpanid(0xDECA);
    dwt_setaddress16(addr);
    dwt_enableframefilter(DWT_FF_DATA_EN);
}

// Virtual functions from dw1000_api

int Decawave::writetospi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 bodylength, const uint8 *bodyBuffer) {
  //_spi.format(8,0);
  //_spi.frequency(2000000);

  chipSelect();

  for (size_t i = 0; i < headerLength; i++)
    _spi->write(headerBuffer[i]);

  for (size_t i = 0; i < bodylength; i++)
    _spi->write(bodyBuffer[i]);

  chipDeselect();

  return 0;
}

int Decawave::readfromspi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 readlength, uint8 *readBuffer) {
  //_spi.format(8,0);
  //_spi.frequency(2000000);

  chipSelect();

  for (size_t i = 0; i < headerLength; i++) {
    readBuffer[0] = _spi->write(headerBuffer[i]);
  }

  for (size_t i = 0; i < readlength; i++) {
    readBuffer[i] = _spi->write(0);
  }

  chipDeselect();

  return 0;
}

decaIrqStatus_t Decawave::decamutexon(void){ return 0;}
void Decawave::decamutexoff(decaIrqStatus_t s){}
void Decawave::deca_sleep(unsigned int time_ms) {
    wait_ms(time_ms);
}


// Callback functions for decawave interrupt

void Decawave::getData_success(const dwt_cb_data_t *cb_data) {
    // uint16 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
    if (FRAME_LEN_MAX <= cb_data->datalength) {
        LOG(WARN, "Frame recieved too large:\r\n"
            "   Recieved: %u Max: %u", cb_data->datalength, FRAME_LEN_MAX);

        rx_status = COMM_DEV_BUF_ERR;
    }
    // Read recived data to rx_buffer array
    dwt_readrxdata(rx_buffer, cb_data->datalength, 0);

    rx_len = cb_data->datalength;
    rx_status = COMM_SUCCESS;
    // LOG(INIT,"almost success");
    // dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

void Decawave::getData_fail(const dwt_cb_data_t *cb_data) {
    if (rx_status != COMM_SUCCESS) { //TODO: probably better way to do this
        rx_status = COMM_DEV_BUF_ERR;
    }
    // dwt_rxenable(DWT_START_RX_IMMEDIATE);
}


// Other functions

void Decawave::logSPI(int num) {
    LOG(INIT, "spi %d %p %p", num, (int)&_spi, *reinterpret_cast<char *>((void*)&_spi));
}

void Decawave::setLED(bool ledOn) {
    dwt_setleds(ledOn);
}

/*
// static void getData_success_cb(const dwt_cb_data_t *cb_data) {
    global_radio->getData_success(cb_data);
}

// static void getData_fail_cb(const dwt_cb_data_t *cb_data) {
    global_radio->getData_fail(cb_data);
}

// int readfromspi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 readlength, uint8 *readBuffer) {
    return global_radio->readfromspi(headerLength, headerBuffer, readlength, readBuffer);
}

// int writetospi(uint16 headerLength, const uint8 *headerBuffer,
                        uint32 bodylength, const uint8 *bodyBuffer) {
    return global_radio->writetospi(headerLength, headerBuffer, bodylength, bodyBuffer);
}
*/