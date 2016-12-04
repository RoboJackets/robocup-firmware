// #include "deca_device_api.h"
#include "Decawave.hpp"

#include "assert.hpp"
#include "logger.hpp"

Decawave* global_radio = nullptr;

/*
Channel(RX/TX usually must be same): varies centre frequency and bandwidth
PRF(pulse repetition frequency)(16 or 64 MHz)(RX/TX must be same): higher PRF more accuracy/range, additional power consumption
    Different PRFs will not be picked up or interfere
Preamble length(TX only): number of symbols repeated in preamble
    Selected based on data rate
    Longer preamble gives improved range but longer air time
PAC(Preamble acquisition chunk) size(RX only): chunk size used to cross correlate to detect preamble
    Determined by expected preamble length
    Larger PAC gives better performance when preamble long enough to allow it
TX preamble code(TX only): used as specification for complex channel
    Allows for multiple unique complex channels on one channel
    Prevents overlapping channels from communicating
    Selection based on the chosen channel and PRF
RX preamble code(RX only): same as the TX preamble code
SFD(standard/non-standard)(RX/TX must be same): 0 for IEEE 802.15.4 standard, 1 for non-standard decawave found to be more robust
Data rate(RX/TX must be same): rate of transmission, higher means faster but less range
PHY header mode(RX/TX must be same): use standard 128 or extended 1024 octet frame
SFD timeout(RX only): timout from start of aquiring preamble to recover from false preamble detection
    Reccomended length: (preamble length + 1 + SFD length - PAC size)
*/

/*
Summary:
    Channel          (choose)
    PRF              (choose)
    Preamble length  (based on data rate)
    PAC size         (based on preamble length)
    TX preamble code (based on channel and PRF)
    RX preamble code (based on TX preamble code)
    SFD              (choose, likely 1)
    Data rate        (choose)
    PHY header mode  (choose, likely STD)
    SFD timout       (based on preamble length)
*/

static dwt_config_t config = {
    4,                 // Channel
    DWT_PRF_64M,       // PRF
    DWT_PLEN_128,      // Preamble length
    DWT_PAC8,          // PAC size
    17,                // TX preamble code
    17,                // RX preamble code
    0,                 // standard/non-standard SFD
    DWT_BR_6M8,        // Data rate
    DWT_PHRMODE_STD,   // PHY header mode
    (128 + 1 + 64 - 8) // SFD timeout
};

static dwt_txconfig_t txconfig = {
    0x95,            // PG delay
    0x9A9A9A9A,      // TX power
};

#define TX_TO_RX_DELAY_UUS 60
#define RX_RESP_TO_UUS 5000

Decawave::Decawave(shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName intPin)
    : CommLink(sharedSPI, nCs, intPin), dw1000_api() {

    global_radio = this; // TODO: This is not good
    setSPIFrequency(2000000); // 2 MHz (lower speed required for config)
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        LOG(FATAL,"Decawave not initialized");
        _isInit = false;
        return;
    }

    reset();
    selfTest();

    if (_isInit) {
        dwt_configure(&config);
        dwt_configuretxrf(&txconfig);

        setSPIFrequency(8000000); // 8 MHz (doesn't work for some reason if higher)

        dwt_setrxaftertxdelay(TX_TO_RX_DELAY_UUS);
        dwt_setcallbacks(NULL, static_cast<dwt_cb_t>(&Decawave::getData_success), NULL, static_cast<dwt_cb_t>(&Decawave::getData_fail));
        dwt_setinterrupt(DWT_INT_RFCG, 1);

        dwt_setautorxreenable(1);

        setLED(true);
        dwt_forcetrxoff(); // TODO: Better way than force off then reset?
        dwt_rxreset();
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        LOG(INIT, "Decawave ready!");
        CommLink::ready();
    }
}

// Virtual functions from CommLink

int32_t Decawave::sendPacket(const rtp::packet* pkt){
    // Return failutre if not initialized
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
    }
    i+=8;
    for (uint8_t byte : pkt->payload) {
        i++;
        tx_buffer[i] = byte;
    }
    tx_buffer[i + 1] = 0;
    tx_buffer[i + 2] = 0;

    dwt_writetxdata(i+3, tx_buffer, 0);
    dwt_writetxfctrl(i+3, 0, 0);

    if (DWT_SUCCESS == dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED)) {
        return COMM_SUCCESS;
    }

    return COMM_DEV_BUF_ERR;
}

void Decawave::printStuff() {
    // printf("n_%#010x\n\r", dwt_read32bitreg(SYS_STATUS_ID));
    // printf("b_%#010x\n\r", dw1000local.cbData.status);
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

    if (rx_status == COMM_SUCCESS) {
        // LOG(INIT, "SUCCESS?");

        for (uint8_t i = 9; i < rx_len - 2; i++) {
            buf->push_back(rx_buffer[i]);
        }
    }

    // Probably not needed
    for (uint8_t i = 0; i < rx_len; i++) {
        rx_buffer[i] = 0;
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
    if (FRAME_LEN_MAX <= cb_data->datalength) {
        LOG(WARN, "Frame recieved too large:\r\n"
            "   Recieved: %u Max: %u", cb_data->datalength, FRAME_LEN_MAX);

        rx_status = COMM_DEV_BUF_ERR;
    }

    // Read recived data to rx_buffer array
    dwt_readrxdata(rx_buffer, cb_data->datalength, 0);

    rx_len = cb_data->datalength;
    rx_status = COMM_SUCCESS;
}

void Decawave::getData_fail(const dwt_cb_data_t *cb_data) {
    if (rx_status != COMM_SUCCESS) { //TODO: probably better way to do this
        rx_status = COMM_DEV_BUF_ERR;
    }
}


// Other functions

void Decawave::logSPI(int num) {
    LOG(INIT, "spi %d %p %p", num, (int)&_spi, *reinterpret_cast<char *>((void*)&_spi));
}

void Decawave::setLED(bool ledOn) {
    dwt_setleds(ledOn);
}
