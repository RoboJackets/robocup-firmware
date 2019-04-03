#include "ISM43340.hpp"
#include "Assert.hpp"
#include "Logger.hpp"
#include <memory>
#include <string>
#include "PinNames.h"

uint32_t mbedPrintWait2 = 50;
ISM43340::ISM43340(SpiPtrT sharedSPI, PinName nCs, PinName _nReset, PinName intPin)
  : CommLink(sharedSPI, nCs, intPin), nReset(_nReset), dataReady(p21) {
    reset();
}

int ISM43340::writeToSpi(uint8_t* command, int length) {

    while (dataReady.read() != 1);

    chipSelect();
    for (int i = 0; i < length; i += 2) {
        if (i < length) {
            uint8_t c1 = command[i];
            uint8_t c2 = 0;
            if (i + 1 < length) {
                c2 = command[i + 1];
            }

            //Swap endianess
            uint16_t packet = (c2 << 8) | c1;

            //Due to the fact the SPI protocol for this takes turns the return can be ignored
            m_spi->write(packet);
        }
    }
    chipDeselect();

    return 0;
}

uint32_t ISM43340::readFromSpi() {

    while (dataReady.read() != 1);

    readBuffer.clear();
    chipSelect();
    while (dataReady.read() != 0) {
        uint16_t data = m_spi->write(0x0a0a);

        //ignore '> ' which is the prompt
        if (data != 0x203e){
            readBuffer.push_back((uint8_t)(data));
            readBuffer.push_back((uint8_t)(data >> 8));
        }
    }
    chipDeselect();

    return 0;
}

void ISM43340::sendCommand(std::string command, std::string arg) {
    command = command + arg;

    // Add delimiter and keep command of even length
    if (command.compare("S3=") != 0) {
        if (command.length() % 2 == 0) {
            command += "\r\n";
        }
        else {
            command += "\r";
        }
    }
    writeToSpi((uint8_t*) command.data(), command.length());
    readFromSpi();
}

int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failure if not initialized
    if (!isInit) return COMM_FAILURE;

    BufferT txBuffer;

    // Reserve memory for AT command header, packet, and delimiter
    int bufferSize = 5 + pkt->size();
    txBuffer.reserve(bufferSize);

    const auto headerFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto headerLastPtr = headerFirstPtr + rtp::HeaderSize;

    const auto cmdFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto cmdLastPtr = headerFirstPtr + rtp::HeaderSize;

    // Insert ISM AT string and payload length
    txBuffer.insert(txBuffer.end(), ISMConstants::CMD_SEND_DATA.begin(), ISMConstants::CMD_SEND_DATA.end());

    char size [5];
    sprintf(size, "%d", pkt->size());

    std::string sizeButAString = size;

    txBuffer.insert(txBuffer.end(), sizeButAString.begin(), sizeButAString.end());
    // Insert \r
    txBuffer.insert(txBuffer.end(), ISMConstants::ODD_DELIMITER.begin(), ISMConstants::ODD_DELIMITER.end());

    // insert the rtp header
    txBuffer.insert(txBuffer.end(), headerFirstPtr, headerLastPtr);
    // insert the rtp payload
    txBuffer.insert(txBuffer.end(), pkt->payload.begin(), pkt->payload.end());

    printf("preparing to send %d bytes\r\n", txBuffer.size());

    for (int i = 0; i < txBuffer.size(); i++) {
      char c = txBuffer[i];
      if (c == '\r') {
        printf("\\r");

      }
      else if(c == '\n') {
        printf("\\n");
      }
      else {
        printf("%c", c);
      }
    }
    printf("\r\n");

    writeToSpi(txBuffer.data(), txBuffer.size());
    readFromSpi();

    return 0;
}

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

int32_t ISM43340::selfTest() {
    // I don't really have anything for this right now
    printf("Starting Self Test\r\n");
    wait_ms(mbedPrintWait2);
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);

    printf("Human Readble, getting connection info\r\n");
    wait_ms(mbedPrintWait2);
    sendCommand(ISMConstants::CMD_GET_CONNECTION_INFO);

    printf("Received Data:\r\n");
    wait_ms(mbedPrintWait2);
    for (int i = 0; i < readBuffer.size(); i++) {
        printf("%c", (char) readBuffer[i]);
    }
    printf("\r\n");
    wait_ms(mbedPrintWait2);


    //This isn't a good measure of correctness
    isInit = readBuffer.size() > 0;

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);

    if (isInit){
        return 0;
    }

    return -1;
}

// Essentially the init function
void ISM43340::reset() {
    nReset = 0;
    nReset = 1;

    setSPIFrequency(6'000'000);

    int i = 0;
    while (dataReady.read() != 1 and i < 100) {
      wait_ms(10);
      i++;
    }

    isInit = dataReady.read();

    if (isInit) {

        readFromSpi();

        // TODO: Check for \r\n in buffer

        //Configure Network
        sendCommand(ISMConstants::CMD_RESET_SOFT);

        sendCommand(ISMConstants::CMD_SET_SSID, "ThePromisedLan");
        sendCommand(ISMConstants::CMD_SET_PASSWORD, "");

        sendCommand(ISMConstants::CMD_SET_SECURITY, "0");

        sendCommand(ISMConstants::CMD_SET_DHCP, "1");


        //Connect to network
        sendCommand(ISMConstants::CMD_JOIN_NETWORK);

        if ((int)readBuffer[0] == 0) {
            //Failed to connect to network
            //not sure what to have it do here
            LOG(INFO, "ISM43340 failed to connect");
            return;
        }

        for (int i = 0; i < readBuffer.size(); i++) {
            printf("%c", readBuffer[i]);
        }

        sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL, "1");
        sendCommand(ISMConstants::CMD_SET_HOST_IP, "192.168.43.252");
        sendCommand(ISMConstants::CMD_SET_PORT, "25565");
        sendCommand(ISMConstants::CMD_START_CLIENT, "1");

        LOG(INFO, "ISM43340 ready!");
        // CommLink::ready();
    }
}
