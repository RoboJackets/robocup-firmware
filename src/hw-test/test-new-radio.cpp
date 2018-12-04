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

uint16_t readBuffer[128];

DigitalIn dataReady(RADIO_DATA_RDY);

inline uint16_t endian(uint16_t num) {
    return (num>>8) | (num<<8);
}

void printGet(SharedSPIDevice<> radioSPI) {
    printf("In printGet\r\n");
    wait_ms(100);

    radioSPI.chipSelect();

    int count = 0;
    uint16_t character = 0;

    while (dataReady.read() != 0) {
        character = radioSPI.m_spi->write('\n');
        readBuffer[count] = character;
        count++;
    }

    radioSPI.chipDeselect();

    for (int i = 0; i < count; i++) {
        printf("%x ", readBuffer[i]);
    }
    printf("\r\n");
    wait_ms(100);
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
    DigitalOut radioRST(RADIO_RST);
    radioRST = 0;
    radioRST = 1;

    printf("%d\r\n", int(dataReady.read()));
    wait_ms(100);

    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    printf("ready\r\n");
    wait_ms(100);

    printGet(radioSPI);

    const std::string showSettings = "C?\n\r";

    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    radioSPI.chipSelect();
    for (uint16_t c : showSettings) {
        uint16_t returns = radioSPI.m_spi->write(endian(c));
        printf("%x ", returns);
    }
    printf("\r\n");
    wait_ms(100);

    radioSPI.chipDeselect();

    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    printGet(radioSPI);

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

    // printf("data: %d\r\n", dataReady.read());

    // // // wait_ms(10);
    // while (dataReady.read() != 1) {
    //     printGet(radioSPI);
    //     wait_ms(1000);
    // }

    return 0;
}
