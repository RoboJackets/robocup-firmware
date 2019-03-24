#include "Assert.hpp"
#include "Logger.hpp"
#include "ISM43340.hpp"
#include <memory>
#include "PinNames.h"

inline uint16_t changeEndianess(uint16_t val) {
    return (val>>8) | (val<<8);
}

uint32_t mbedPrintWait2 = 50;
ISM43340::ISM43340(SpiPtrT sharedSPI, PinName nCs, PinName _nReset, PinName intPin)
  : CommLink(sharedSPI, nCs, intPin), nReset(_nReset), dataReady(p21) {
    reset();
}

int ISM43340::writeToSpi(uint8_t* command, int length) {

  while (dataReady.read() != 1) {
  }


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

  //what am I returning here? probably a status code I guess
  return 0;
}

uint32_t ISM43340::readFromSpi() {

    while (dataReady.read() != 1) {
    }

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

    //is this a status again? this is weird
    return 0;
}

void ISM43340::sendCommand(std::string command, std::string arg) {
    command = command + arg;

    // Add delimiter and keep command of even length
    if (command.length() % 2 == 0) {
        command += "\r\n";
    }
    else {
        command += "\r";
    }

    writeToSpi((uint8_t*) command.data(), command.length());
    readFromSpi();
}


/* COMMAND SET DEFINITIONS */



// //Check prompt
// inline void checkPrompt () {
//     // read the initial prompt into a buffer
//     readfromspi();
//     // check the buffer for the response sequence of the prompt (Not the same as the OK)
//     readbuffer
//     // you could probably have read spi return an int as a status code for both cant decide
// }

// // This command will probably not be used but is here as an example command
// // Set Human Readable
// inline setHumanReadable () {
//     std::string compositeString(CMD_SET_HUMAN_READABLE);
//     // write composite string to the radio
//     writetospi(&compositeString);
//     // read to global buffer
//     readfromspi();
// }



//Set Machine Readable

//Connect to access point

//Set SSID

//Set network security type

//Set DHCP usage

//Disconnect from network

//Show connection status

//Send Data TCP


// // This one is here as an example as well
// //Show network settings
// inline showNetworkSettings() {
//     std::string compositeString(CMD_SHOW_NETWORK_SETTINGS);

//     writetospi(&compositeString);
//     readfromspi();
//     // the returned buffer from this is of unknown length. However it is well bounded by the length of possible network information
// }


int32_t ISM43340::sendPacket(const rtp::Packet* pkt) {
    // Return failure if not initialized
    if (!isInit) return COMM_FAILURE;

    BufferT txBuffer;

    const auto bufferSize = 3 + pkt->size() + 2;
    txBuffer.reserve(bufferSize);

    const auto headerFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto headerLastPtr = headerFirstPtr + rtp::HeaderSize;

    const auto cmdFirstPtr = reinterpret_cast<const uint8_t*>(&pkt->header);
    const auto cmdLastPtr = headerFirstPtr + rtp::HeaderSize;

    // insert the rtp header
    txBuffer.insert(txBuffer.end(), headerFirstPtr, headerLastPtr);
    // insert the rtp payload
    txBuffer.insert(txBuffer.end(), pkt->payload.begin(), pkt->payload.end());

    // ISM Stuff
    txBuffer.insert(txBuffer.begin(), ISMConstants::CMD_SEND_DATA.begin(), ISMConstants::CMD_SEND_DATA.end());

    // Add delimiter and keep command of even length


    if (txBuffer.size() % 2 == 0) {
        txBuffer.push_back('\r');
        txBuffer.push_back('\n');
    }
    else {
        txBuffer.push_back('\r');
    }
    writeToSpi(txBuffer.data(), txBuffer.size());

    //is this a status?
    return 0;
}

void ISM43340::setAddress(int addr) {
    //I have no clue what this does
}

int32_t ISM43340::selfTest() {
    //I don't really have anything for this right now
    printf("Starting Self Test\r\n");
    wait_ms(mbedPrintWait2);
    sendCommand(ISMConstants::CMD_SET_HUMAN_READABLE);
    readFromSpi();

    printf("Human Readble, getting connection info\r\n");
    wait_ms(mbedPrintWait2);
    sendCommand(ISMConstants::CMD_GET_CONNECTION_INFO);
    readFromSpi();

    printf("Received Data:\r\n");
    wait_ms(mbedPrintWait2);
    for (int i = 0; i < readBuffer.size(); i++) {
        printf("%c", (char) readBuffer[i]);
        wait_ms(mbedPrintWait2);
    }
    printf("\r\n");
    wait_ms(mbedPrintWait2);


    //This isn't a good measure of correctness
    isInit = readBuffer.size() > 0;

    sendCommand(ISMConstants::CMD_SET_MACHINE_READABLE);
    readFromSpi();

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

        readFromSpi();

        //Configure Network
        sendCommand(ISMConstants::CMD_RESET_SOFT);

        sendCommand(ISMConstants::CMD_SET_SSID, "rjwifi");
        sendCommand(ISMConstants::CMD_SET_PASSWORD, "61E880222C");

        sendCommand(ISMConstants::CMD_SET_SECURITY, "3");

        sendCommand(ISMConstants::CMD_SET_DHCP, "1");


        //Connect to network
        sendCommand(ISMConstants::CMD_JOIN_NETWORK);

        if ((int)readBuffer[0] == 0) {
            //Failed to connect to network
            //not sure what to have it do here
            LOG(INFO, "ISM43340 failed to connect");
            return;
        }

        printf("joined\r\n");

        sendCommand(ISMConstants::CMD_SET_TRANSPORT_PROTOCOL, "0");

        sendCommand(ISMConstants::CMD_SET_HOST_IP, "192.168.1.108");

        sendCommand(ISMConstants::CMD_SET_PORT, "25565");

        sendCommand(ISMConstants::CMD_START_CLIENT, "1");

        LOG(INFO, "ISM43340 ready!");
        CommLink::ready();
    }
}
