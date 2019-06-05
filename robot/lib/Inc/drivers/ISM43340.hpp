#pragma once

#include "mtrain.hpp"
#include "SPI.hpp"
#include <vector>
#include <memory>

#include "rc-fshare/rtp.hpp"

#define LOG_DEBG "DEBG"
#define LOG_INFO "INFO"
#define LOG_WARN "WARN"
#define LOG_ERRO "ERRO"

#define LOG_MAIN  "MAIN"
#define LOG_RADIO "RADIO"

#define LOG(level, module, f_, ...) { \
        HAL_Delay(10); \
        printf("[%s] %s :: ", level, module); \
        fflush(stdout); \
        HAL_Delay(10); \
        printf((f_), ##__VA_ARGS__); \
        fflush(stdout); \
        HAL_Delay(10); \
    }

class ISM43340 {
public:

    ISM43340(std::shared_ptr<SPI> radioSPI, PinName nCsPin, PinName nResetPin, PinName dataReadyPin);

    //I'm assuming we are still using RTP packets
    int32_t sendPacket(const rtp::Packet* pkt);

    std::vector<uint8_t> getData();

    std::vector<uint8_t> getReadBuffer() { return readBuffer; }

    void reset();

    int32_t selfTest();

    void pingRouter();

    bool isConnected() const { return isInit; }

    int writeToSpi(uint8_t* command, int length);

    int32_t testPrint();

    uint32_t readFromSpi();

    //I could add a status return to this but meh
    void sendCommand(std::string command, std::string arg = "");

private:

    std::shared_ptr<SPI> radioSPI;

    DigitalOut nCs;
    DigitalOut nReset;
    DigitalIn dataReady;

    std::vector<uint8_t> readBuffer;

    bool isInit = false;
};
