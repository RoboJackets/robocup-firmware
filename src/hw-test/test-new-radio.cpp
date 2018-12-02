// #define __CM3_REV 0x200

#include <mbed.h>
#include <vector>
#include "Logger.hpp"
#include "Rtos.hpp"
// #include <cmsis_os.h>
#include "SharedSPI.hpp"

#include "PinNames.h"
#include "RobotDevices.hpp"

#define RJ_ENABLE_ROBOT_CONSOLE

const PinName RADIO_SPI_MOSI = p5;
const PinName RADIO_SPI_MISO = p6;
const PinName RADIO_SPI_SCK = p7;
const PinName RADIO_SPI_NCS = p20;
const PinName RADIO_DATA_RDY = p21;
const PinName RADIO_RST = p22;

uint8_t readBuffer[128];

void printGet(SharedSPIDevice<> radioSPI) {
    radioSPI.chipSelect();

    int count = 0;
    uint16_t character = 0;

    while (character != 0x15 || count % 2) {
        character = radioSPI.m_spi->write('\n');
        readBuffer[count] = character;
        count++;
    }

    radioSPI.chipDeselect();

    for (int i = 0; i < count; i++) {
        printf("%x ", readBuffer[i]);
    }
    printf("\r\n");
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
    spiBus->frequency(6'000'000);

    auto radioSPI = SharedSPIDevice<>(spiBus, RADIO_SPI_NCS, true);

    DigitalIn dataReady(RADIO_DATA_RDY);

    DigitalOut radioRST(RADIO_RST);
    radioRST = 0;
    radioRST = 1;

    while (dataReady.read() != 1) {
        wait_ms(10);
    }
    printf("ready\r\n");

    printGet(radioSPI);

    // while (dataReady.read() != 1) {
    //     wait_ms(10);
    // }

    // const std::string showSettings = "C?\n\r";

    // radioSPI.chipSelect();
    // // for (char c : showSettings) {
    // //     uint8_t returns = radioSPI.m_spi->write(c);
    // //     printf("%x ", returns);
    // // }
    // // printf("\r\n");

    // uint16_t returns = radioSPI.m_spi->write(('C' << 8) + '?');
    // printf("%x ", returns);
    // returns = radioSPI.m_spi->write(('\n' << 8) + '\r');
    // printf("%x ", returns);
    // printf("\r\n");

    // // while (dataReady.read() != 1) {
    // //     uint8_t character = 0;
    // //     character = radioSPI.m_spi->write('\n');
    // //     printf("%x ", character);
    // //     // fflush(stdout);
    // //     wait_ms(100);
    // // }
    // // printf("\r\n");

    // radioSPI.chipDeselect();

    // printf("data: %d\r\n", dataReady.read());

    // // // wait_ms(10);
    // while (dataReady.read() != 1) {
    //     printGet(radioSPI);
    //     wait_ms(1000);
    // }

    return 0;
}