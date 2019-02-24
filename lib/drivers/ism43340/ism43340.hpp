#pragma once

#include "CommLink.hpp"

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

    virtual int writetospi(char* command, int commandLength);

    virtual int readfromspi(uint8* readbuffer, uint32* readlength);

    void setLED(bool ledOn) { dwt_setleds(ledOn); };

private:
    InterruptIn dataReady();

    BufferPtrT m_rxBufferPtr = nullptr;
    BufferPtrT m_txBufferPtr = nullptr;
    uint32_t m_chipVersion;
    bool m_isInit = false;
    DigitalOut nReset;

    void getDataSuccess(const dwt_cb_data_t* cb_data);
    void getDataFail(const dwt_cb_data_t* cb_data);
};
