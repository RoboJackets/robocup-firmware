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

uint16_t readBuffer[512];

DigitalIn dataReady(RADIO_DATA_RDY);
DigitalOut ssn(RADIO_SPI_NCS);

inline uint16_t endian(uint16_t val) {
    return (val>>8) | (val<<8);
}

uint32_t mbedPrintWait = 50;

void receiveData(SharedSPIDevice<> radioSPI) {
    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    printf("Data Phase\r\n");
    wait_ms(mbedPrintWait);

    int count = 0;
    uint16_t character = 0;

    radioSPI.chipSelect();
    while (dataReady.read() != 0) {
        character = radioSPI.m_spi->write(0x0a0a);
        readBuffer[count] = endian(character);
        count++;
    }
    radioSPI.chipDeselect();

    printf("Received Data: ");
    for (int i = 0; i < count; i++) {
        printf("%04x ", readBuffer[i]);
    }
    printf("\r\n");
    wait_ms(mbedPrintWait);
}

void sendCmd(SharedSPIDevice<> radioSPI, char* command) {
    while (dataReady.read() != 1) {
        wait_ms(10);
    }

    printf("Command Phase\r\n");
    wait_ms(mbedPrintWait);

    int length = strlen(command);

    radioSPI.chipSelect();
    char lastC = 0;
    for (int i = 0; i < length; i++) {
        char c = command[i];
        if (lastC != 0) {
            // NOTE:check memory space
            uint16_t data = (c << 8) | lastC;
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
        wait_ms(10);
    }

    printf("Booted: %d\r\n", int(dataReady.read()));
    wait_ms(mbedPrintWait);

    // data phase (prompt)
    receiveData(radioSPI);

    char cmdReset[] = "ZR\r\n";
    // command phase
    sendCmd(radioSPI, cmdReset);

    // data phase
    receiveData(radioSPI);


    char cmdSSID[] = "C1=rjwifi\r";
    // command phase
    sendCmd(radioSPI, cmdSSID);

    // data phase
    receiveData(radioSPI);


    char cmdSetPassword[] = "C2=61E880222C\r";
    // command phase
    sendCmd(radioSPI, cmdSetPassword);

    // data phase
    receiveData(radioSPI);


    char cmdSetSecurity[] = "C3=4\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetSecurity);

    // data phase
    receiveData(radioSPI);


    char cmdSetDHCP[] = "C4=1\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetDHCP);

    // data phase
    receiveData(radioSPI);


    char cmdSetHumanReadable[] = "$$$\r";
    // command phase
    sendCmd(radioSPI, cmdSetHumanReadable);

    // data phase
    receiveData(radioSPI);


    char cmdConnectionInfo[] = "C?\r\n";
    // command phase
    sendCmd(radioSPI, cmdConnectionInfo);

    // data phase
    receiveData(radioSPI);

    char cmdSetMachineReadable[] = "---\r";
    // command phase
    sendCmd(radioSPI, cmdSetMachineReadable);

    // data phase
    receiveData(radioSPI);

    char cmdJoinNetwork[] = "C0\r\n";
    // command phase
    sendCmd(radioSPI, cmdJoinNetwork);

    // data phase
    receiveData(radioSPI);

    if ((int)readBuffer[0] == 0) {
		printf("ERROR: Failed to join network\n");
		return(-1);
    }

    //PINGING
    // char cmdSetPingTarget[] = "T1=192.168.1.1\r\n";
    // // command phase
    // sendCmd(radioSPI, cmdSetPingTarget);

    // // data phase
    // receiveData(radioSPI);


    // char cmdShowPingSettings[] = "T?\r\n";
    // // command phase
    // sendCmd(radioSPI, cmdShowPingSettings);

    // // data phase
    // receiveData(radioSPI);


    // char cmdPing[] = "T0\r\n";
    // // command phase
    // sendCmd(radioSPI, cmdPing);

    // // data phase
    // receiveData(radioSPI);

    //TCP!!!
    char cmdSetProtocol[] = "P1=0\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetProtocol);

    // data phase
    receiveData(radioSPI);

    char cmdJoinNetwork[] = "P3=";
    // command phase
    sendCmd(radioSPI, cmdJoinNetwork);

    // data phase
    receiveData(radioSPI);

    printf("Done\r\n");
    return 0;
}
