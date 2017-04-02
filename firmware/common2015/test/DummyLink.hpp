#pragma once

#include "CommLink.hpp"

class DummyLink : public CommLink {
public:
    DummyLink(SpiPtrT sharedSPI, PinName nCs, PinName intPin);

    virtual int32_t sendPacket(const RTP::Packet* pkt) override { return 0; }

    virtual BufferT getData() override { return BufferT{0}; }

    virtual void reset() override { return; }

    virtual int32_t selfTest() override { return 0; }

    virtual bool isConnected() const override { return true; }

    virtual void setAddress(int addr) override { return; }
};
