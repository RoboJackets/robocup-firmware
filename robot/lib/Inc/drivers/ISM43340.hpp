#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>
#include "rc-fshare/rtp.hpp"

namespace ISMConstants {
    static const std::string OK = "OK\r\n\>";
    static const std::string PROMPT = "\r\n>";

    static const std::string EVEN_DELIMITER = "\r\n";
    static const std::string ODD_DELIMITER = "\r";

    static const std::string EMPTY_TRANSFER = "\n\n";

    static const std::string CMD_SET_HUMAN_READABLE = "$$$";
    static const std::string CMD_SET_MACHINE_READABLE = "---";
    static const std::string CMD_RESET_FACTORY = "Z0";
    static const std::string CMD_RESET_SOFT = "ZR";

    static const std::string CMD_JOIN_NETWORK = "C0";
    static const std::string CMD_SET_SSID = "C1=";
    static const std::string CMD_SET_PASSWORD = "C2=";
    static const std::string CMD_SET_SECURITY = "C3=";
    static const std::string CMD_SET_DHCP = "C4=";
    static const std::string CMD_GET_CONNECTION_INFO = "C?";

    static const std::string CMD_GET_RADIO_SOCKET = "P0";
    static const std::string CMD_SET_RADIO_SOCKET = "P0=";
    static const std::string CMD_SET_TRANSPORT_PROTOCOL = "P1=";
    static const std::string CMD_SET_HOST_IP = "P3=";
    static const std::string CMD_SET_PORT = "P4=";
    static const std::string CMD_START_SERVER = "P5=";
    static const std::string CMD_START_CLIENT = "P6=";

    static const std::string CMD_SEND_DATA = "S3=";

    static const std::string CMD_RECEIVE_DATA = "R0";
    static const std::string CMD_SET_READ_SIZE = "R1=";
};


class ISM43340 {
public:

    ISM43340(SPI radioSPI, PinName nCsPin, PinName nResetPin, PinName dataReadyPin);

    //I'm assuming we are still using RTP packets
    int32_t sendPacket(const rtp::Packet* pkt);

    std::vector<uint8_t> getData();

    std::vector<uint8_t> getReadBuffer() { return readBuffer; }

    void reset();

    int32_t selfTest();

    bool isConnected() const { return isInit; }

    int writeToSpi(uint8_t* command, int length);

    //Printing isn't implemented  yet
    //int32_t testPrint();

    uint32_t readFromSpi();

    //I could add a status return to this but meh
    void sendCommand(std::string command, std::string arg = "");

private:

    SPI radioSPI;

    DigitalIn dataReady;
    DigitalOut nCs;

    DigitalOut nReset;
    //Need to convert to use interrupts
    //InterruptIn dataReady();

    std::vector<uint8_t> readBuffer;

    bool isInit = false;
};
