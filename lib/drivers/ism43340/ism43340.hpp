#pragma once

#include "CommLink.hpp"

class Ism43340 : public CommLink {
public:
    Ism43340(SpiPtrT sharedSPI, PinName nCs, PinName intPin, PinName _nReset);

    virtual int32_t sendPacket(const rtp::Packet* pkt) override;

    virtual BufferT getData() override;

    // virtual void reset() override { dwt_softreset(); }
    virtual void reset() override;

    virtual int32_t selfTest() override;

    virtual bool isConnected() const override { return m_isInit; }

    virtual void setAddress(int addr) override;

    virtual int writetospi(uint16 headerLength, const uint8* headerBuffer,
                           uint32 bodylength, const uint8* bodyBuffer) override;
    virtual int readfromspi(uint16 headerLength, const uint8* headerBuffer,
                            uint32 readlength, uint8* readBuffer) override;

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
