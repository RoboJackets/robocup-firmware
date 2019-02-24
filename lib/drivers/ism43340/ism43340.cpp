#include "Assert.hpp"
#include "Logger.hpp"

#include <memory>

#define CMD_SET_HUMAN_READABLE "$$$"
#define CMD_SET_MACHINE_READABLE "---"
#define CMD_RESET_SETTINGS "Z0"

#define CMD_JOIN_NETWORK "C0"
#define CMD_SET_SSID "C1="
#define CMD_SET_PASSWORD "C2="
#define CMD_SET_SECURITY "C3="
#define CMD_SET_DHCP "C4="
#define CMD_GET_CONNECTION_INFO "C?"

#define CMD_SET_TRANSPORT_PROTOCOL "P1="
#define CMD_SET_HOST_IP "P3="
#define CMD_SET_PORT "P4="
#define CMD_START_CLIENT "P6="

#define CMD_SEND_DATA "S3="

inline uint16_t swapendian(uint16_t val) {
    return (val>>8) | (val<<8);
}

ISM43340::ISM43340(SpiPtrT sharedSPI, PinName nCs, PinName intPin,
                   PinName _nReset)
    : CommLink(sharedSPI, nCs, intPin), nReset(_nReset) {
    reset();
}

int ISM43340::writetospi(char* command, int commandLength) {
    // pad command if needed
    if (commandLength % 2 == 0) {
        strcat(command, "\r\n");
    } else {
        strcat(command, "\r");
    }

    // endianness conversion from the char array. It is cast to 16 bits
    for (uint32_t i = 0; i < commandLength; i + 2) {
        *((uint16_t*)command[i]) = endian(*((uint16_t*)command[i]));
    }


    // TODO: implement spi transfer from SPI.cpp

    // Original write function
    // uint8_t lastc = 0;
    // radiospi.chipselect();
    // for (int i = 0; i < command->size(); i++) {
    //     uint8_t c = command[i];
    //     if (lastc != 0) {
    //         uint16_t packet = (c << 8) | lastc;
    //         radiospi.m_spi->write(packet);
    //         lastc = 0;
    //     } else {
    //         lastc = c;
    //     }
    // }
    radiospi.chipdeselect();
    return 0;
}

int ISM43340::readfromspi(uint8* readbuffer, uint32* readlength) {
    while (dataready.read() != 1) {
    }

    packetcount = 0;
    radiospi.chipselect();
    while (dataready.read() != 0) {
        uint16_t data = radiospi.m_spi->write(0x0a0a);
        readbuffer[packetcount] = (uint8_t)(data);
        readbuffer[packetcount + 1] = (uint8_t)(data >> 8);
        packetcount += 2;
    }
    radiospi.chipdeselect();
    return 0;
}


/* COMMAND SET DEFINITIONS */

//Check prompt
inline checkPrompt () {
    // read the initial prompt into a buffer
    readfromspi();
    // check the buffer for the response sequence of the prompt (Not the same as the OK)
    // you could probably have read spi return an int as a status code for both cant decide
}

// This command will probably not be used but is here as an example command
// Set Human Readable
inline setHumanReadable() {
    std::string compositeString(CMD_SET_HUMAN_READABLE);

    // write composite string to the radio
    writetospi(&compositeString);
    // read to global buffer
    readfromspi();
}

//Set Machine Readable

//Connect to access point

//Set SSID

//Set network security type

//Set DHCP usage

//Disconnect from network

//Show connection status

// This one is here as an example as well
//Show network settings
inline showNetworkSettings() {
    std::string compositeString(CMD_SHOW_NETWORK_SETTINGS);

    // formats command adding /r and /n when needed
    formatCommand(&compositeString);

    // write composite string to the radio
    writetospi(&compositeString);
    // read to global buffer. This needs to be returned to the user
    readfromspi();
    // the returned buffer from this is of unknown length. However it is well bounded by the length of possible network information
}


int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failutre if not initialized
    if (!m_isInit) return COMM_FAILURE;


    // dont know how much of this needs to be kept. Soccer may actually want the split between body and head of the sent packet. I dont know
    BufferT txBuffer;

    const auto bufferSize = 9 + pkt->size() + 2;
    txBuffer.reserve(bufferSize);

    // MAC layer header for Decawave
    txBuffer.insert(txBuffer.end(),
                    {0x41, 0x88, 0x00, 0xCA, 0xDE, pkt->header.address, 0x00,
                     static_cast<uint8_t>(m_address), 0x00});

    ASSERT(txBuffer.size() == 9);

    const auto headerFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto headerLastPtr = headerFirstPtr + rtp::HeaderSize;

    // insert the rtp header
    txBuffer.insert(txBuffer.end(), headerFirstPtr, headerLastPtr);
    // insert the rtp payload
    txBuffer.insert(txBuffer.end(), pkt->payload.begin(), pkt->payload.end());
    // insert padding for CRC
    txBuffer.insert(txBuffer.end(), {0x00, 0x00});

    /* This is the Decawave's send buffer code
    dwt_writetxdata(txBuffer.size(), txBuffer.data(), 0);
    dwt_writetxfctrl(txBuffer.size(), 0, 0);
    */
    // For us we simply need the original buffer and the original buffer length
    // We then have to convert it to a string and add the command start (S3=N\rDATA) where N the original buffer length and DATA is the original buffer as a string
    // The SPI write will need the final length of the command Including the length to represent n and be properly padded

    // This error code can be replaced with the return of calling send transport data
    const auto sentStatus =
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    const auto hadError = sentStatus != DWT_SUCCESS;

    return hadError ? COMM_DEV_BUF_ERR : COMM_SUCCESS;
}

// No clue how this works
CommLink::BufferT ISM43340::getData() {
    BufferT buf{};

    // Return empty data if not initialized
    if (!m_isInit) return std::move(buf);

    // set the m_rxBuffer's pointer to our vector before calling the isr
    // function
    m_rxBufferPtr = &buf;

    // manually invoke the isr routine & set back into RX mode
    dwt_isr();
    // our buffer is now filled with the received bytes if everything went ok
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    // return empty buffer if the isr routine failed to fill it with anything
    if (buf.empty()) return std::move(buf);

    // remove the last 2 elements
    buf.erase(buf.end() - 2, buf.end());

    // move the buffer to the caller
    return std::move(buf);
}

int32_t ISM43340::selfTest() {
    // ISM read device number
    m_chipVersion = dwt_readdevid();

    // Compare return buffer from read device id
    if (m_chipVersion != DWT_DEVICE_ID) {
        LOG(SEVERE,
            "Decawave part number error:\r\n"
            "    Found:\t0x%02X (expected 0x%02X)",
            m_chipVersion, DWT_DEVICE_ID);

        return -1;
    } else {
        m_isInit = true;
        return 0;
    }
}

// Essentially the init function
void ISM43340::reset() {
    nReset = 0;
    wait_ms(3);
    nReset = 1;

    m_isInit = 0;
    setSPIFrequency(2'000'000);

    // check for prompt
    if () {
        LOG(SEVERE, "ISM43340 not initialized");
        return;
    }

    // Attempt retrieval of device id
    selfTest();

    if (m_isInit) {
        // Access point config from hwtest
        // char cmdSSID[] = "C1=rjwifi\r";
        // char cmdSetPassword[] = "C2=61E880222C\r";
        // char cmdSetSecurity[] = "C3=3\r\n";
        // char cmdSetDHCP[] = "C4=1\r\n";
        // char cmdJoinNetwork[] = "C0\r\n";

        // TCP config from hwtest
        // char cmdSetProtocol[] = "P1=0\r\n";
        // char cmdSetHostIP[] = "P3=192.168.1.108\r\n";
        // char cmdSetPort[] = "P4=25565\r\n";
        // char cmdStartClient[] = "P6=1\r\n";

        // I dont think any of the rest of this except the log and CommLink::ready() matter
        dwt_configure(&config);
        dwt_configuretxrf(&txconfig);

        setSPIFrequency(6'000'000);  // won't work after 8MHz

        dwt_setrxaftertxdelay(TX_TO_RX_DELAY_UUS);
        dwt_setcallbacks(
            nullptr, static_cast<dwt_cb_t>(&Decawave::getDataSuccess), nullptr,
            static_cast<dwt_cb_t>(&Decawave::getDataFail));
        dwt_setinterrupt(DWT_INT_RFCG, 1);

        dwt_setautorxreenable(1);

        setLED(true);
        //
        dwt_forcetrxoff();  // TODO: Better way than force off then reset?
        dwt_rxreset();

        LOG(INFO, "ISM43340 ready!");
        CommLink::ready();
    }
