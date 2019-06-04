#include "drivers/ISM43340.hpp"

ISM43340::ISM43340(SPI radioSPI, PinName nCsPin, PinName nResetPin,
                   PinName dataReadyPin)
    : radioSPI(radioSPI),
      nCs(nCsPin),
      nReset(nResetPin),
      dataReady(dataReadyPin) {
    nCs = ISMConstants::CHIP_DESELECT;

    reset();
}

int ISM43340::writeToSpi(uint8_t* command, int length) {
    HAL_Delay(ISMConstants::SPI_DELAY);
    while (dataReady.read() != 1);
    HAL_Delay(ISMConstants::SPI_DELAY);

    nCs = ISMConstants::CHIP_SELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    for (int i = 0; i < length; i += 2) {
        uint8_t c1 = command[i];
        uint8_t c2 = 0;
        if (i + 1 < length) {
            c2 = command[i + 1];
        }

        // Swap endianess
        radioSPI.transmit(c2);
        radioSPI.transmit(c1);
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    return 0;
}

uint32_t ISM43340::readFromSpi() {
    unsigned int bytesRead = 0;

    readBuffer.clear();

    HAL_Delay(ISMConstants::SPI_DELAY);
    while (dataReady.read() != 1);
    HAL_Delay(ISMConstants::SPI_DELAY);
    
    nCs = ISMConstants::CHIP_SELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    while (dataReady.read() == 1) {
        uint8_t data1 = radioSPI.transmitReceive(ISMConstants::READ);
        uint8_t data2 = radioSPI.transmitReceive(ISMConstants::READ);

        // Read only if both bytes are NACK's and
        // the receive buffer isn't full
        if (!(data1 == ISMConstants::NACK &&
              data2 == ISMConstants::NACK) &&
            readBuffer.size() < ISMConstants::RECEIVE_BUFF_SIZE) {

            // Swap endianess so readBuff is correct order
            readBuffer.push_back(data2);
            readBuffer.push_back(data1);
            bytesRead += 2;
        }
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    return bytesRead;
}

void ISM43340::sendCommand(std::string command, std::string arg) {
    // Add delimiter and keep command of even length
    if (command.compare("S3=") != 0) {
        command = command + arg;
        if (command.length() % 2 == 0) {
            command += ISMConstants::EVEN_DELIMITER;
        } else {
            command += ISMConstants::ODD_DELIMITER;
        }
    } else {
        command = command + arg;
    }

    writeToSpi((uint8_t*)command.data(), command.length());
    readFromSpi();
}

int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failure if not initialized
    if (!isInit) return -1;

    std::vector<uint8_t> txBuffer;

    // Transmit using socket #1
    // Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "1");

    // Reserve memory for AT command header, packet, and delimiter
    int bufferSize = 5 + pkt->size();
    txBuffer.reserve(bufferSize);

    const auto headerFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto headerLastPtr = headerFirstPtr + rtp::HeaderSize;

    const auto cmdFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto cmdLastPtr = headerFirstPtr + rtp::HeaderSize;

    // Insert ISM AT string and payload length
    txBuffer.insert(txBuffer.end(), ISMConstants::CMD_SEND_DATA.begin(),
                    ISMConstants::CMD_SEND_DATA.end());

    char size[5];
    sprintf(size, "%d", pkt->size());

    std::string sizeButAString = size;

    txBuffer.insert(txBuffer.end(), sizeButAString.begin(),
                    sizeButAString.end());
    // Insert \r
    txBuffer.insert(txBuffer.end(), ISMConstants::ODD_DELIMITER.begin(),
                    ISMConstants::ODD_DELIMITER.end());

    // insert the rtp header
    txBuffer.insert(txBuffer.end(), headerFirstPtr, headerLastPtr);
    // insert the rtp payload
    txBuffer.insert(txBuffer.end(), pkt->payload.begin(), pkt->payload.end());

    printf("preparing to send %d bytes\r\n", txBuffer.size());

    for (int i = 0; i < txBuffer.size(); i++) {
        char c = txBuffer[i];
        if (c == '\r') {
            printf("\\r");
        } else if (c == '\n') {
            printf("\\n");
        } else {
            printf("%c", c);
        }
    }
    printf("\r\n");

    writeToSpi(txBuffer.data(), txBuffer.size());
    readFromSpi();

    return 0;
}

std::vector<uint8_t> ISM43340::getData() {
    // Receive using socket #0
    // Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, "0");

    // Should add some sort of check to see if data is ready to be read here
    sendCommand(ISMConstants::CMD_READ_TRANSPORT_DATA);

    return readBuffer;
}

/* printf not implemented yet
int32_t ISM43340::testPrint() {
    printf("Readbuffer Contains:\r\n");
    wait_ms(mbedPrintWait2);
    for (int i = 0; i < readBuffer.size(); i++) {
        printf("%c", (char) readBuffer[i]);
    }
    printf("\r\n");
    wait_ms(mbedPrintWait2);
    return 0;
}
*/

int32_t ISM43340::selfTest() {
    // I don't really have anything for this right now
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);

    sendCommand(ISMConstants::CMD_GET_CONNECTION_INFO);

    // This isn't a good measure of correctness
    isInit = readBuffer.size() > 0;

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);

    if (isInit) {
        return 0;
    }

    return -1;
}

void ISM43340::pingRouter() {
    LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_TARGET_ADDRESS, "172.16.1.1");

    LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_REPEAT_COUNT, "10");

    LOG(LOG_INFO, LOG_RADIO, "CMD_SET_PING_DELAY\r\n");
    sendCommand(ISMConstants::CMD_SET_PING_DELAY, "10");

    LOG(LOG_INFO, LOG_RADIO, "CMD_PING_TARGET_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_PING_TARGET_ADDRESS);
}

// Essentially the init function
void ISM43340::reset() {
    LOG(LOG_INFO, LOG_RADIO, "Resetting\r\n");

    nReset = ISMConstants::RESET_TURN_OFF;
    HAL_Delay(ISMConstants::RESET_DELAY);
    nReset = ISMConstants::RESET_TURN_ON;
    HAL_Delay(ISMConstants::RESET_DELAY);

    radioSPI.frequency(ISMConstants::SPI_FREQ);

    // Wait for device to turn on
    int i = 0;
    while (dataReady.read() != 1 and i < 100) {
        HAL_Delay(10);
        i++;
    }

    if (!dataReady.read()) {
        LOG(LOG_ERRO, LOG_RADIO, "Could not initialize radio");
        return;
    }

    // Get the first prompt
    readFromSpi();

    // Send invalid command
    // This is needed to get the device to connect to a network???
    //LOG(LOG_INFO, LOG_RADIO, "CMD_INVALID\r\n");
    sendCommand(ISMConstants::CMD_INVALID);

    // Configure Network

    // Disconnect from a network if power doesn't toggle
    //LOG(LOG_INFO, LOG_RADIO, "CMD_DISCONNECT_NETWORK\r\n");
    sendCommand(ISMConstants::CMD_DISCONNECT_NETWORK);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SSID\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SSID,
                "rj-rc-field");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_PASSWORD\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_PASSWORD,
                "r0b0jackets");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SECURITY_TYPE\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SECURITY_TYPE,
                ISMConstants::TYPE_NETWORK_SECURITY::WPA2_AES);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_DHCP\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_DHCP,
                ISMConstants::TYPE_NETWORK_DHCP::ENABLED);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_IP_VERSION\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_IP_VERSION,
                ISMConstants::TYPE_NETWORK_IP_VERSION::IPV4);

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_GATEWAY\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_GATEWAY,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_PRIMARY_DNS\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_PRIMARY_DNS,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_SECONDARY_DNS\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_SECONDARY_DNS,
                "172.16.1.1");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_NETWORK_JOIN_RETRY_COUNT\r\n");
    sendCommand(ISMConstants::CMD_SET_NETWORK_JOIN_RETRY_COUNT,
                "3");

    //LOG(LOG_INFO, LOG_RADIO, "CMD_NETWORK_AUTO_CONNECT\r\n");
    sendCommand(ISMConstants::CMD_NETWORK_AUTO_CONNECT,
                ISMConstants::TYPE_NETWORK_AUTO_CONNECT::AUTO_JOIN_RECONNECT);

    // Connect to network
    //LOG(LOG_INFO, LOG_RADIO, "CMD_JOIN_NETWORK\r\n");
    sendCommand(ISMConstants::CMD_JOIN_NETWORK);

    if (readBuffer.size() == 0 || (int)readBuffer[0] == 0) {
        // Failed to connect to network
        // not sure what to have it do here

        LOG(LOG_INFO, LOG_RADIO, "Failed to connect to network\r\n");
        return;
    }

    // Port initialization
    // UDP receive
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_COMMUNICATION_SOCKET\r\n");
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET,
                "0");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_PROTOCOL\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                "172.16.1.22");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                "25566");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_START_CLIENT\r\n");
    sendCommand(ISMConstants::CMD_START_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    // UDP send
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_RADIO_SOCKET\r\n");
    sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "1");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_PROTOCOL\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                "172.16.1.22");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER\r\n");
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                "25565");
    
    //LOG(LOG_INFO, LOG_RADIO, "CMD_START_CLIENT\r\n");
    sendCommand(ISMConstants::CMD_START_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    LOG(LOG_INFO, LOG_RADIO, "Radio initialized");
}
