#include "drivers/ISM43340.hpp"

ISM43340::ISM43340(SPI radioSPI, PinName nCsPin, PinName nResetPin, PinName dataReadyPin) :
  radioSPI(radioSPI), nCs(nCsPin), nReset(nResetPin), dataReady(dataReadyPin){
    //This should really be implemented
    //this->radioSPI.format(16);

    reset();
}

// 8 bit incorrect version
int ISM43340::writeToSpi(uint8_t* command, int length) {

  while (dataReady.read() != 1);
  for (int i = 0; i < length; i += 2) {
    if (i < length) {
      uint8_t c1 = command[i];
      uint8_t c2 = 0;
      if (i + 1 < length) {
        c2 = command[i + 1];
      }

      //Swap endianess
      uint16_t packet = (c2 << 8) | c1;

      radioSPI.transmit(c2);
      radioSPI.transmit(c1);
    }
  }

  return 0;
}

/* 16 BIT CORRECT VERSION
int ISM43340::writeToSpi(uint8_t* command, int length) {

    while (dataReady.read() != 1);
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
            radioSPI.transmit(packet);
        }
    }

    return 0;
}
*/


// 8 bit incorrect version
uint32_t ISM43340::readFromSpi() {

  while (dataReady.read() != 1);

  readBuffer.clear();

  while (dataReady.read() != 0) {
      uint8_t data1 = radioSPI.transmitReceive(0x0a);
      uint8_t data2 = radioSPI.transmitReceive(0x0a);

      readBuffer.push_back(data2);
      readBuffer.push_back(data1);
  }

  return 0;
}

/* 16 BIT CORRECT VERSION
uint32_t ISM43340::readFromSpi() {

    while (dataReady.read() != 1);

    readBuffer.clear();

    while (dataReady.read() != 0) {
        uint16_t data = radioSPI.transmitReceive(0x0a0a);

        //ignore '> ' which is the prompt
        if (data != 0x203e){
            readBuffer.push_back((uint8_t)(data));
            readBuffer.push_back((uint8_t)(data >> 8));
        }
    }

    return 0;
}
*/

void ISM43340::sendCommand(std::string command, std::string arg) {

    // Add delimiter and keep command of even length
    if (command.compare("S3=") != 0) {
        command = command + arg;
        if (command.length() % 2 == 0) {
        command += "\r\n";
        }
        else {
            command += "\r";
        }
    }
    else {
        command = command + arg;
    }
    writeToSpi((uint8_t*) command.data(), command.length());
    readFromSpi();
}

int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failure if not initialized
    if (!isInit) return -1;

    std::vector<uint8_t> txBuffer;

    //Transmit using socket #1
    //Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "1");


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


std::vector<uint8_t> ISM43340::getData() {
    //Receive using socket #0
    //Could increase efficiency by tracking if this needs to be set
    sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "0");

    //Should add some sort of check to see if data is ready to be read here
    sendCommand(ISMConstants::CMD_RECEIVE_DATA);

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

    radioSPI.frequency(6'000'000);

    int i = 0;
    while (dataReady.read() != 1 and i < 100) {
        HAL_Delay(10);
        i++;
    }

    isInit = dataReady.read();

    if (isInit) {

        readFromSpi();

        // TODO: Check for \r\n in buffer

        //Configure Network
        sendCommand(ISMConstants::CMD_RESET_SOFT);

        sendCommand(ISMConstants::CMD_SET_SSID, "rj-rc-field");
        sendCommand(ISMConstants::CMD_SET_PASSWORD, "r0b0jackets");

        sendCommand(ISMConstants::CMD_SET_SECURITY, "4");

        sendCommand(ISMConstants::CMD_SET_DHCP, "1");

        //Connect to network
        sendCommand(ISMConstants::CMD_JOIN_NETWORK);

        if (readBuffer.size() == 0 || (int)readBuffer[0] == 0) {
            //Failed to connect to network
            //not sure what to have it do here

            //LOG(INFO, "ISM43340 failed to connect"); //not sure logging is implemented yet
            return;
        }

        sendCommand(ISMConstants::CMD_GET_RADIO_SOCKET);


        // Port initialization
        // UDP receive
        sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "0");
        sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL, "1");
        sendCommand(ISMConstants::CMD_SET_HOST_IP, "172.16.1.22");
        sendCommand(ISMConstants::CMD_SET_PORT, "25566");
        sendCommand(ISMConstants::CMD_START_CLIENT, "1");

        // UDP send
        sendCommand(ISMConstants::CMD_SET_RADIO_SOCKET, "1");
        sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL, "1");
        sendCommand(ISMConstants::CMD_SET_HOST_IP, "172.16.1.22");
        sendCommand(ISMConstants::CMD_SET_PORT, "25565");
        sendCommand(ISMConstants::CMD_START_CLIENT, "1");


        //is logging implemented yet?
        //LOG(INFO, "ISM43340 ready!");
        }
}