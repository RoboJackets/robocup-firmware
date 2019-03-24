#pragma once

#include "CommLink.hpp"
#include "ISM43340_api.hpp"


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

    static const std::string CMD_SET_TRANSPORT_PROTOCOL = "P1=";
    static const std::string CMD_SET_HOST_IP = "P3=";
    static const std::string CMD_SET_PORT = "P4=";
    static const std::string CMD_START_CLIENT = "P6=";

    static const std::string CMD_SEND_DATA = "S3=";
}

class ISM43340 : public CommLink {
public:
  ISM43340(SpiPtrT sharedSPI, PinName nCs,  PinName _nReset, PinName intPin = NC);

    int32_t sendPacket(const rtp::Packet* pkt) override;

    BufferT getData() { return readBuffer; }

    void reset() override;

    int32_t selfTest() override;

    bool isConnected() const override { return isInit; }

    void setAddress(int addr) override;

    int writeToSpi(uint8_t* command, int length);

    uint32_t readFromSpi();

    //I could add a status return to this but meh
    void sendCommand(std::string command, std::string arg = "");
private:


    DigitalIn dataReady;
    //Pretty sure this should just be a input pin
    //InterruptIn dataReady();

    BufferT readBuffer;

    uint32_t m_chipVersion;
    bool isInit = false;
    DigitalOut nReset;

    void getDataSuccess(const dwt_cb_data_t* cb_data);
    void getDataFail(const dwt_cb_data_t* cb_data);

};
