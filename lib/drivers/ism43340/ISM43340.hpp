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
    static const std::string CMD_RESET_SETTINGS = "Z0";

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
    ISM43340(SpiPtrT sharedSPI, PinName nCs, PinName intPin, PinName _nReset);

    virtual int32_t sendPacket(const rtp::Packet* pkt) override;

    virtual BufferT getData() override;

    // virtual void reset() override { dwt_softreset(); }
    virtual void reset() override;

    virtual int32_t selfTest() override;

    virtual bool isConnected() const override { return m_isInit; }

    virtual void setAddress(int addr) override;

    virtual int writeToSpi(char* command, int commandLength);

    virtual uint32_t readFromSpi(uint8* readbuffer, uint32* readlength);

    virtual void fmtCmd(BufferT& command, BufferT& payload);

  //Not sure I need this
  //void setLED(bool ledOn) { dwt_setleds(ledOn); };

private:


    DigitalIn dataReady;
    //Pretty sure this should just be a input pin
    //InterruptIn dataReady();

    BufferPtrT m_rxBufferPtr = nullptr;
    BufferPtrT m_txBufferPtr = nullptr;
    uint32_t m_chipVersion;
    bool m_isInit = false;
    DigitalOut nReset;

    void getDataSuccess(const dwt_cb_data_t* cb_data);
    void getDataFail(const dwt_cb_data_t* cb_data);
};
