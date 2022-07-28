#include "drivers/ISM43340.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "FreeRTOS.h"
#include "delay.h"
#include "interrupt_in.h"
#include "macro.hpp"
#include "task.h"

volatile ISMConstants::State currentState;

// Callback on every state change on data ready
void dataReady_cb() {
    // lmao
    // this should be obvious
    //
    // In all seriousness, this is just a state machine where we always
    // increment to the next state whenever there is an edge trigger
    // This is basically the `++x % y` that you see
    currentState =
        static_cast<ISMConstants::State>((static_cast<uint8_t>(currentState) + 1) %
                                         static_cast<uint8_t>(ISMConstants::State::NumStates));
}

ISM43340::ISM43340(std::unique_ptr<SPI> radioSPI, PinName nCsPin, PinName nResetPin,
                   PinName dataReadyPin)
    : radioSPI(std::move(radioSPI)),
      nCs(nCsPin),
      nReset(nResetPin),
      dataReady{dataReadyPin.port, dataReadyPin.pin},
      currentSocket(SOCKET_TYPE::SEND),
      cmdStart(nullptr) {
    currentState = ISMConstants::State::CommandReady;
    interruptin_init_ex(dataReady, &dataReady_cb, PULL_DOWN, INTERRUPT_RISING_FALLING);

    nCs = ISMConstants::CHIP_DESELECT;

    readBuffer.reserve(ISMConstants::RECEIVE_BUFF_SIZE);

    reset();
}

bool ISM43340::isAvailable() {
    // See if we are already on the correct socket before doing stuff
    if (currentSocket != SOCKET_TYPE::RECEIVE) {
        sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, ISMConstants::RECEIVE_SOCKET);
        currentSocket = SOCKET_TYPE::RECEIVE;
    }

    // Try to read data from device
    // If there is no data, device returns "\r\n\r\nOK\r\n> "
    // If there is data, device returns raw data
    // printf("Is there stuff to read? ");
    sendCommand(ISMConstants::CMD_READ_TRANSPORT_DATA);

    // Check to see if it's the normal prompt
    std::string noData = "\r\n\r\nOK\r\n> ";
    std::string err = "-1";

    bool matchNoData = true;
    for (unsigned int i = 0; i < noData.size(); i++) {
        if (i >= readBuffer.size()) {
            matchNoData = false;
            break;
        } else if (readBuffer.at(i) != noData.at(i)) {
            matchNoData = false;
            break;
        }
    }

    bool matchErr = true;
    for (unsigned int i = 0; i < err.size(); i++) {
        if (i >= readBuffer.size()) {
            matchErr = false;
            break;
        } else if (readBuffer.at(i) != err.at(i)) {
            matchErr = false;
            break;
        }
    }

    return !matchNoData && !matchErr;
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
    unsigned int numDigitsWritten =
        snprintf(reinterpret_cast<char*>(&arg[0]), maxNumDigits, "%d", numBytes);

    // Copy delimiter
    memcpy(&arg[numDigitsWritten], &ISMConstants::ODD_DELIMITER[0], oddDelimiterSize);

    // Copy data
    memcpy(&arg[numDigitsWritten + oddDelimiterSize], &data[0], numBytes);

    // Send it
    sendCommand(ISMConstants::CMD_WRITE_TRANSPORT_DATA_INLINE, &arg[0], argSize);

    return numBytes;
}

unsigned int ISM43340::receive(uint8_t* data, const unsigned int maxNumBytes) {
    // There's an extra 0x0a 0x0d on the front of the packet
    unsigned int amntToCopy = readBuffer.size() - 2;

    // limit to their buffer size
    if (amntToCopy > maxNumBytes) {
        amntToCopy = maxNumBytes;
    }

    memcpy(data, readBuffer.data() + 2, amntToCopy);

    return amntToCopy;
}

/**
 * Note: All delays in the function below have been tuned
 * to work correctly. The data sheet misses some key ones
 * and the ones given are partially wrong
 *
 * Bump the super loop call rate until it just barely works
 * Drop these delays until it just barely works
 * Bump the super loop call rate again until it works
 * repeat
 *
 * Let it run for a minute or so to see if it breaks eventually
 */
void ISM43340::writeToSpi(uint8_t* command, int length) {
    while (currentState != ISMConstants::State::CommandReady)
        ;

    nCs = ISMConstants::CHIP_SELECT;
    delay_from_microseconds(
        100);  // Must be 50 us or more. Measure first response on logic analyzer

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

    nCs = ISMConstants::CHIP_DESELECT;

    // Wait till data ready goes to 0
    while (currentState != ISMConstants::State::CommandAck)
        ;
}

uint32_t ISM43340::readFromSpi() {
    unsigned int bytesRead = 0;

    readBuffer.clear();

    // Wait till data ready goes to 1
    while (currentState != ISMConstants::State::ResponseReady)
        ;

    nCs = ISMConstants::CHIP_SELECT;
    delay_from_microseconds(
        100);  // Must be 50 us or more. Measure first response on logic analyzer

    // Once we find any data on the bus
    // 0x25 0x25 is a valid character combination in the packet
    bool foundAnyData = false;

    while (currentState != ISMConstants::State::ResponseDone) {
        uint8_t data1 = radioSPI->transmitReceive(ISMConstants::READ);
        uint8_t data2 = radioSPI->transmitReceive(ISMConstants::READ);

        // If we read two nacks and we haven't found data yet, quit out
        // otherwise, check if the buffer is full and read it in
        if (!(data1 == ISMConstants::NACK && data2 == ISMConstants::NACK && !foundAnyData) &&
            readBuffer.size() < ISMConstants::RECEIVE_BUFF_SIZE) {
            // Swap endianess so readBuff is correct order
            readBuffer.push_back(data2);
            readBuffer.push_back(data1);
            bytesRead += 2;

            foundAnyData = true;
        }
    }

    nCs = ISMConstants::CHIP_DESELECT;

    return bytesRead;
}

void ISM43340::sendCommand(const std::string& command, const uint8_t* arg,
                           const unsigned int argSize) {
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
            memcpy(&cmdStart[totalLength], &ISMConstants::EVEN_DELIMITER[0],
                   ISMConstants::EVEN_DELIMITER.length());
            totalLength += ISMConstants::EVEN_DELIMITER.length();
        } else {
            memcpy(&cmdStart[totalLength], &ISMConstants::ODD_DELIMITER[0],
                   ISMConstants::ODD_DELIMITER.length());
            totalLength += ISMConstants::ODD_DELIMITER.length();
        }
    }

    writeToSpi(&cmdStart[0], totalLength);
    readFromSpi();
}

void ISM43340::sendCommand(const std::string& command, const std::string& arg) {
    sendCommand(command, reinterpret_cast<const uint8_t*>(arg.data()), arg.length());
}

int32_t ISM43340::testPrint() {
    for (unsigned int i = 0; i < readBuffer.size(); i++) {
        printf("[INFO] %c", (char)readBuffer[i]);
        vTaskDelay(10);
    }
    printf("\r\n");
    vTaskDelay(50);
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

    sendCommand(ISMConstants::CMD_SET_PING_TARGET_ADDRESS, ISMConstants::ROUTER_IP);
    sendCommand(ISMConstants::CMD_SET_PING_DELAY, "10");
    sendCommand(ISMConstants::CMD_SET_PING_REPEAT_COUNT, "10");
    sendCommand(ISMConstants::CMD_PING_TARGET_ADDRESS);
    testPrint();

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);
}

void ISM43340::reset() {
    for (int i = 0; i < 5; i++) {
        nReset = ISMConstants::RESET_TURN_OFF;
        delay_from_tick(ISMConstants::RESET_DELAY);
        nReset = ISMConstants::RESET_TURN_ON;
        delay_from_tick(ISMConstants::RESET_DELAY);
        radioSPI->frequency(ISMConstants::SPI_FREQ);

        // Wait for device to turn on
        for (int counter = 0; counter < 1500; counter++) {
            delay_from_tick(10);
            if (interruptin_read(dataReady)) {
                break;
            }
        }

        isInit = interruptin_read(dataReady);

        if (isInit) {
            break;
        } else if (i == 4) {
            printf("[ERROR] Failed to initialize radio.\r\n");
        } else {
            printf("[INFO] Could not initialize radio. Retrying.\r\n");
        }
    }

    nCs = ISMConstants::CHIP_SELECT;
    DWT_Delay(500);

    // Get the first prompt
    currentState = ISMConstants::State::ResponseReady;
    readFromSpi();

    // Send invalid command
    // This is needed to get the device to connect to a network???
    sendCommand(ISMConstants::CMD_INVALID);

    sendCommand(ISMConstants::CMD_POWER_MANAGEMENT, "0");

    // Configure Network

    // Disconnect from a network if power doesn't toggle
    sendCommand(ISMConstants::CMD_DISCONNECT_NETWORK);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SSID, ISMConstants::NETWORK_SSID);

    sendCommand(ISMConstants::CMD_SET_NETWORK_PASSWORD, ISMConstants::NETWORK_PASSWORD);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SECURITY_TYPE,
                ISMConstants::TYPE_NETWORK_SECURITY::WPA2_AES);

    sendCommand(ISMConstants::CMD_SET_NETWORK_DHCP, ISMConstants::TYPE_NETWORK_DHCP::ENABLED);

    sendCommand(ISMConstants::CMD_SET_NETWORK_IP_VERSION,
                ISMConstants::TYPE_NETWORK_IP_VERSION::IPV4);

    sendCommand(ISMConstants::CMD_SET_NETWORK_GATEWAY, ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_PRIMARY_DNS, ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_SECONDARY_DNS, ISMConstants::ROUTER_IP);

    sendCommand(ISMConstants::CMD_SET_NETWORK_JOIN_RETRY_COUNT, "3");

    sendCommand(ISMConstants::CMD_NETWORK_AUTO_CONNECT,
                ISMConstants::TYPE_NETWORK_AUTO_CONNECT::AUTO_JOIN_RECONNECT);

    // Connect to network
    sendCommand(ISMConstants::CMD_JOIN_NETWORK);

    if (readBuffer.size() == 0 || (int)readBuffer[0] == 0) {
        // Failed to connect to network
        // not sure what to have it do here
        printf("[ERROR] Failed to connect to network\r\n");
        return;
    }

    // Port initialization
    // UDP receive

    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, ISMConstants::RECEIVE_SOCKET);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                ISMConstants::BASE_STATION_IP);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER, ISMConstants::LOCAL_PORT);

    sendCommand(ISMConstants::CMD_SET_READ_TRANSPORT_PACKET_SIZE, "12");

    sendCommand(ISMConstants::CMD_SET_READ_TRANSPORT_TIMEOUT, "1");

    sendCommand(ISMConstants::CMD_SET_RECEIVE_MODE, "0");

    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    // UDP send

    sendCommand(ISMConstants::CMD_SET_COMMUNICATION_SOCKET, ISMConstants::SEND_SOCKET);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL,
                ISMConstants::TYPE_TRANSPORT_PROTOCOL::UDP_ENABLED);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_HOST_IP_ADDRESS,
                ISMConstants::BASE_STATION_IP);

    sendCommand(ISMConstants::CMD_SET_TRANSPORT_REMOTE_PORT_NUMBER,
                ISMConstants::BASE_STATION_PORT);

    sendCommand(ISMConstants::CMD_START_STOP_TRANSPORT_CLIENT,
                ISMConstants::TYPE_TRANSPORT_CLIENT::ENABLE);

    currentSocket = SOCKET_TYPE::SEND;
    connected = true;
    printf("[INFO] Radio initialized\r\n");
}
