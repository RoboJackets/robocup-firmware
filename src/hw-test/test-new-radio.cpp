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
DigitalOut ssn(RADIO_SPI_NCS);

inline uint16_t endian(uint16_t val) {
    return (val>>8) | (val<<8);
}

uint32_t mbedPrintWait = 50;

void printGet(SharedSPIDevice<> radioSPI) {
    printf("Data Phase\r\n");
    wait_ms(mbedPrintWait);

    radioSPI.chipSelect();

    int count = 0;
    uint16_t character = 0;

    while (dataReady.read() != 0) {
        character = radioSPI.m_spi->write(0x0a0a);
        readBuffer[count] = endian(character);
        count++;
    }

    radioSPI.chipDeselect();

    printf("Received Data: ");
    for (int i = 0; i < count; i++) {
        printf("%x ", readBuffer[i]);
    }
    printf("\r\n");
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
    spiBus->frequency(115200);

    auto radioSPI = SharedSPIDevice<>(spiBus, RADIO_SPI_NCS, true);
    DigitalOut radioRST(RADIO_RST);
    radioRST = 0;
    radioRST = 1;

    printf("\r\n");
    wait_ms(mbedPrintWait);

    while (dataReady.read() != 1) {
        // wait_ms(10);
    }

    printf("Booted: %d\r\n", int(dataReady.read()));
    wait_ms(mbedPrintWait);

    // data phase (prompt)
    printGet(radioSPI);

    // command phase
    char* command = "AS=0,ABC\r\n";

    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    // printf("Command Phase\r\n");
    // wait_ms(mbedPrintWait);

    radioSPI.chipSelect();
    uint8_t lastC = 0;
    for (uint8_t i = 0; i < 10; i++) {
        char c = command[i];
        if (lastC != 0) {
            // NOTE:check memory space
            uint16_t data = (c << 8) | lastC;
            printf("Data to be sent: %x,%x\r\n", c, lastC);
            wait_ms(mbedPrintWait);
            uint16_t returns = endian(radioSPI.m_spi->write(data)); // data write doesnt have to be flipped because of the order we merged it. Returns does.
            printf("%x ", returns);
            lastC = 0;
        } else {
            lastC = c;
        }
    }
    radioSPI.chipDeselect();

    printf("\r\n");
    wait_ms(mbedPrintWait);

    // end command phase

    printf("ssn: %d\r\n", int(ssn.read()));
    wait_ms(mbedPrintWait);

    printf("data ready: %d\r\n", int(dataReady.read()));
    wait_ms(mbedPrintWait);

    while (dataReady.read() != 1) {
        // wait_ms(10);
    }

    printf("data ready: %d\r\n", int(dataReady.read()));
    wait_ms(mbedPrintWait);

    // data phase
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
