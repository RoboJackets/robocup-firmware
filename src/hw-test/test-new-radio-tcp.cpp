#include <mbed.h>
#include <vector>
#include "Logger.hpp"
#include "Rtos.hpp"
// #include <cmsis_os.h>
#include "SharedSPI.hpp"
#include <chrono>

#include "PinNames.h"
#include "RobotDevices.hpp"
#include "time.h"

#include "ISM43340.hpp"

#define RJ_ENABLE_ROBOT_CONSOLE

const PinName RADIO_SPI_MOSI = p5;
const PinName RADIO_SPI_MISO = p6;
const PinName RADIO_SPI_SCK = p7;
const PinName RADIO_SPI_NCS = p20;
const PinName RADIO_DATA_RDY = p21;
const PinName RADIO_RST = p22;

inline uint16_t endian(uint16_t val) {
    return (val>>8) | (val<<8);
}

uint8_t readBuffer[1024];
int packetCount = 0;

Timer timer;

DigitalIn dataReady(RADIO_DATA_RDY);
DigitalOut ssn(RADIO_SPI_NCS);

uint32_t mbedPrintWait = 100;

void printReadBuffer(std::vector<uint8_t> readBuffer) {
    printf("Received Data: ");
    for (int i = 0; i < packetCount; i++) {
        printf("%x", readBuffer[i]);
    }
    printf("\r\n");

    printf("Byte Count: %d", packetCount);

    wait_ms(mbedPrintWait);
}

int main() {
    const auto mainID = Thread::gettid();
    ASSERT(mainID != nullptr);
    {
        // clear any extraneous rx serial bytes
        Serial s(RJ_SERIAL_RXTX);
        while (s.readable()) s.getc();

        // set baud rate to higher value than the default for faster terminal
        s.baud(57600);
    }

    auto spiBus = std::make_shared<SharedSPI>(RADIO_SPI_MOSI, RADIO_SPI_MISO, RADIO_SPI_SCK);
    spiBus->format(16, 0);  // 16 bits per transfer
    //spiBus->frequency(115200);
    spiBus->frequency(6'000'000);

    // auto radioSPI = std::make_shared<SharedSPIDevice>(spiBus, RADIO_SPI_NCS, true);

    printf("MBed Starting\r\n");
    wait_ms(mbedPrintWait);

    ISM43340 radioDriver(spiBus, RADIO_SPI_NCS, RADIO_RST, RADIO_DATA_RDY);

    printf("ISM43340 Constructed\r\n");
    wait_ms(mbedPrintWait);

    // bool booted = false;
    // int attempt = 0;
    // while (!booted){
    //   wait_ms(10);
    //   // printf("attempt #%d\r\n", attempt);
    //   // wait_ms(mbedPrintWait);
    //   radioDriver.reset();
    //   booted = radioDriver.isConnected();
    //   attempt += 1;
    // }

    printf("reset complete\r\n");
    printf("does this make it print if I say more things\r\n");
    wait_ms(mbedPrintWait);

    bool test = radioDriver.selfTest();
    printf("Self Test: %b", test);
    wait_ms(mbedPrintWait);

    timer.reset();
    for (int iter = 0; iter < 10; iter++) {
        timer.start();

        //Transport data
        std::string cmdSendData = "42\r012345678901234567890123456789012345678912";
        radioDriver.sendCommand("S3=", cmdSendData);
        // command phase
        printf("Sent Data #%d\r\n", iter);

        timer.stop();
        printf("Micro Seconds Elapsed: %d\r\n", timer.read_us());
        wait_ms(mbedPrintWait);
        timer.reset();
    }

    printf("Done\r\n");
    return 0;
}
