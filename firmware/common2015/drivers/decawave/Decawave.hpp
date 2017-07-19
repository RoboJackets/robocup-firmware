#pragma once

#include "CommLink.hpp"
#include "decadriver/deca_device_api.hpp"

class Decawave : public CommLink, public dw1000_api {
public:
    Decawave(SpiPtrT sharedSPI, PinName nCs, PinName intPin);

    virtual int32_t sendPacket(const rtp::Packet* pkt) override;

    virtual BufferT getData() override;

    virtual void reset() override { dwt_softreset(); }

    virtual int32_t selfTest() override;

    virtual bool isConnected() const override { return m_isInit; }

    virtual void setAddress(int addr) override;

    virtual int writetospi(uint16 headerLength, const uint8* headerBuffer,
                           uint32 bodylength, const uint8* bodyBuffer) override;
    virtual int readfromspi(uint16 headerLength, const uint8* headerBuffer,
                            uint32 readlength, uint8* readBuffer) override;
    virtual decaIrqStatus_t decamutexon() override { return 0; }

    void setLED(bool ledOn) { dwt_setleds(ledOn); };

private:
    BufferPtrT m_rxBufferPtr = nullptr;
    BufferPtrT m_txBufferPtr = nullptr;
    uint32_t m_chipVersion;
    bool m_isInit = false;

    void getDataSuccess(const dwt_cb_data_t* cb_data);
    void getDataFail(const dwt_cb_data_t* cb_data);
};
