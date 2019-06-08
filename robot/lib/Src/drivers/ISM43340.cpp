#include "drivers/ISM43340.hpp"
#include <cstring>

ISM43340::ISM43340(std::unique_ptr<SPI> radioSPI, PinName nCsPin, PinName nResetPin,
                   PinName dataReadyPin)
    : radioSPI(std::move(radioSPI)),
      nCs(nCsPin),
      nReset(nResetPin),
      dataReady(dataReadyPin),
      currentSocket(SOCKET_TYPE::SEND),
      cmdStart(nullptr) {

    nCs = ISMConstants::CHIP_DESELECT;

    readBuffer.reserve(ISMConstants::RECEIVE_BUFF_SIZE);

    reset();
}

bool ISM43340::isAvailable() {
    // See if we are already on the correct socket before doing stuff
    if (currentSocket != SOCKET_TYPE::RECEIVE) {
        sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET,
                    ISMConstants::RECEIVE_SOCKET);
        currentSocket = SOCKET_TYPE::RECEIVE;
    }

    // Try to read data from device
    // If there is no data, device returns "\r\n\r\nOK\r\n> "
    // If there is data, device returns raw data
    sendCommand(ISMConstants::CMD_READ_TRANSPORT_DATA);

    // todo fix me. Assumption that all data received is at least 10 bytes    
    return readBuffer.size() <= 10;
}

unsigned int ISM43340::send(const uint8_t* data, const unsigned int numBytes) {
    // Move to send socket if we're not already there
    if (currentSocket != SOCKET_TYPE::SEND) {
        sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, ISMConstants::SEND_SOCKET);
        currentSocket = SOCKET_TYPE::SEND;
    }

    // Since this is the special command in the format "S3=NUM_BYTES<CR>DATA"
    // we need to format arg to be NUM_BYTES<CR>DATA
    // Try's to avoid all dynamic memory as well

    // Max number of digits to the size of the packet being sent over TCP/UDP
    const unsigned int maxNumDigits = 5;
    const unsigned int oddDelimiterSize = ISMConstants::ODD_DELIMITER.size();
    const unsigned int argSize = numBytes + maxNumDigits + oddDelimiterSize;
    uint8_t arg[argSize];

    // Num digits written, not including null termination
    unsigned int numDigitsWritten = snprintf(reinterpret_cast<char*>(&arg[0]), maxNumDigits, "%d", numBytes);

    // Copy delimiter
    memcpy(&arg[numDigitsWritten], &ISMConstants::ODD_DELIMITER[0], oddDelimiterSize);

    // Copy data
    memcpy(&arg[numDigitsWritten + oddDelimiterSize], &data[0], numBytes);

    // Send it
    sendCommand(ISMConstants::CMD_WRITE_TRANSPORT_DATA_INLINE, &arg[0], argSize);

    return numBytes;
}

unsigned int ISM43340::receive(uint8_t* data, const unsigned int maxNumBytes) {
    unsigned int amntToCopy = readBuffer.size();
    
    // limit to their buffer size
    if (amntToCopy > maxNumBytes) {
        amntToCopy = maxNumBytes;
    }

    // TODO: check that data in readBuffer is raw packet data
    // with nothing extra
    memcpy(&data[0], &readBuffer[0], amntToCopy);

    return amntToCopy;
}

void ISM43340::writeToSpi(uint8_t* command, int length) {
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
        radioSPI->transmit(c2);
        radioSPI->transmit(c1);
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);
}

uint32_t ISM43340::readFromSpi() {
    unsigned int bytesRead = 0;

    readBuffer.clear();

    HAL_Delay(ISMConstants::SPI_DELAY);
    while (dataReady.read() != 1);
    HAL_Delay(ISMConstants::SPI_DELAY);
    
    nCs = ISMConstants::CHIP_SELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    // Once we find any data on the bus
    // 0x25 0x25 is a valid character combination in the packet
    bool foundAnyData = false;

    while (dataReady.read() == 1) {
        uint8_t data1 = radioSPI->transmitReceive(ISMConstants::READ);
        uint8_t data2 = radioSPI->transmitReceive(ISMConstants::READ);

        // If we read two nacks and we haven't found data yet, quit out
        // otherwise, check if the buffer is full and read it in
        if (!(data1 == ISMConstants::NACK &&
              data2 == ISMConstants::NACK &&
              !foundAnyData) &&
            readBuffer.size() < ISMConstants::RECEIVE_BUFF_SIZE) {

            // Swap endianess so readBuff is correct order
            readBuffer.push_back(data2);
            readBuffer.push_back(data1);
            bytesRead += 2;

            foundAnyData = true;
        }
    }

    HAL_Delay(ISMConstants::SPI_DELAY);
    nCs = ISMConstants::CHIP_DESELECT;
    HAL_Delay(ISMConstants::SPI_DELAY);

    return bytesRead;
}

void ISM43340::sendCommand(const std::string& command, const uint8_t *arg, const unsigned int argSize) {
    unsigned int totalLength = 0;

    // Add to the cmd struct backwards so the end of the cmd touches the arg
    // This allows for partial indexing into the cmd struct through the entire arg
    for (int i = command.length() - 1; i >= 0; i--) {
        // For 3 char command, 3 - 3 = 0
        // For 2 char command, 3 - 2 = 1
        // For 1 char command, 3 - 1 = 2
        uint8_t argIdxOffset = ISMConstants::MAX_CMD_SIZE - command.length();
        builtCommand.cmd[i + argIdxOffset] = command[i];

        // Always point to last cmd char written
        // Should always be first char in cmd since we're going backwards in list
        cmdStart = &builtCommand.cmd[i + argIdxOffset];
    }
    totalLength += command.length();

    // TODO: make sure argsize is smaller than our buffer size

    // Copy arg into command struct
    memcpy(&builtCommand.arg[0], &arg[0], argSize);
    totalLength += argSize;

    // Append the odd or even delimiter depending on built command length
    //
    // Make sure not the special command which has a different format
    // "S3=XXX<CR>XXX"
    // All other commands use the format
    // "XX=XXXX<CR>" or "XX<CR>"
    if (command.compare("S3=") != 0) {
        if (totalLength % 2 == 0) {
            memcpy(&cmdStart[totalLength],
                   &ISMConstants::EVEN_DELIMITER[0],
                   ISMConstants::EVEN_DELIMITER.length());
            totalLength += ISMConstants::EVEN_DELIMITER.length();
        } else {
            memcpy(&cmdStart[totalLength],
                   &ISMConstants::ODD_DELIMITER[0],
                   ISMConstants::ODD_DELIMITER.length());
            totalLength += ISMConstants::ODD_DELIMITER.length();
        }
    }

    writeToSpi(&cmdStart[0], totalLength);
    readFromSpi();
}

void ISM43340::sendCommand(const std::string& command, const std::string& arg) {
    sendCommand(command,
                reinterpret_cast<const uint8_t*>(arg.data()),
                arg.length());
}

int32_t ISM43340::testPrint() {
    for (unsigned int i = 0; i < readBuffer.size(); i++) {
        printf("%c", (char) readBuffer[i]);
        HAL_Delay(10);
    }
    printf("\r\n");
    HAL_Delay(50);
    return 0;
}

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
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);

    sendCommand(ISMConstants::CMD_SET_PING_TARGET_ADDRESS, "172.16.1.1");
    sendCommand(ISMConstants::CMD_SET_PING_DELAY, "10");
    sendCommand(ISMConstants::CMD_SET_PING_REPEAT_COUNT, "10");
    sendCommand(ISMConstants::CMD_PING_TARGET_ADDRESS);
    testPrint();
    
    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);
}

void ISM43340::reset() {
    nReset = ISMConstants::RESET_TURN_OFF;
    HAL_Delay(ISMConstants::RESET_DELAY);
    nReset = ISMConstants::RESET_TURN_ON;
    HAL_Delay(ISMConstants::RESET_DELAY);

    radioSPI->frequency(ISMConstants::SPI_FREQ);

    // Wait for device to turn on
    int i = 0;
    while (dataReady.read() != 1 && i < 100) {
        HAL_Delay(10);
        i++;
    }

    isInit = !dataReady.read();

    if (isInit) {
        printf("Could not initialize radio\r\n");
        return;
    }

    // Get the first prompt
    readFromSpi();

    // Send invalid command
    // This is needed to get the device to connect to a network???
    sendCommand(ISMConstants::CMD_INVALID);

    sendCommand(ISMConstants::CMD_POWER_MANAGEMENT, "0");


    // Configure Network

    // Disconnect from a network if power doesn't toggle
    sendCommand(ISMConstants::CMD_DISCONNECT_NETWORK);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SSID,
                ISMConstants::NETWORK_SSID);

    sendCommand(ISMConstants::CMD_SET_NETWORK_PASSWORD,
                ISMConstants::NETWORK_PASSWORD);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SECURITY_TYPE,
                ISMConstants::TYPE_NETWORK_SECURITY::WPA2_AES);

    sendCommand(ISMConstants::CMD_SET_NETWORK_DHCP,
                ISMConstants::TYPE_NETWORK_DHCP::ENABLED);

    sendCommand(ISMConstants::CMD_SET_NETWORK_IP_VERSION,
                ISMConstants::TYPE_NETWORK_IP_VERSION::IPV4);

    sendCommand(ISMConstants::CMD_SET_NETWORK_GATEWAY,
                ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_PRIMARY_DNS,
                ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SECONDARY_DNS,
                ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_JOIN_RETRY_COUNT,
                "3");

    sendCommand(ISMConstants::CMD_NETWORK_AUTO_CONNECT,
                ISMConstants::TYPE_NETWORK_AUTO_CONNECT::AUTO_JOIN_RECONNECT);

    // Connect to network
    sendCommand(ISMConstants::CMD_JOIN_NETWORK);

    if (readBuffer.size() == 0 || (int)readBuffer[0] == 0) {
        // Failed to connect to network
        // not sure what to have it do here

        printf("Failed to connect to network\r\n");
        return;
    }

    // Port initialization
    // UDP receive
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET,
                ISMConstants::RECEIVE_SOCKET);
    
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                ISMConstants::BASE_STATION_IP);
    
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                ISMConstants::LOCAL_PORT);
    
    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    // UDP send
    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET,
                ISMConstants::SEND_SOCKET);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);
    
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                ISMConstants::BASE_STATION_IP);
    
    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                ISMConstants::BASE_STATION_PORT);
    
    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    currentSocket = SOCKET_TYPE::SEND;

    // todo set read timeout

    printf("Radio initialized\r\n");
}
