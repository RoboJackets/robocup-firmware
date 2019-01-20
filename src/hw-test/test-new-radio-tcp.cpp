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

uint32_t mbedPrintWait = 50;

void receiveData(SharedSPIDevice<> radioSPI) {
    while (dataReady.read() != 1) {
    }

    // printf("Data Phase\r\n");
    // wait_ms(mbedPrintWait);

    packetCount = 0;
    radioSPI.chipSelect();
    while (dataReady.read() != 0) {
        uint16_t data = radioSPI.m_spi->write(0x0a0a);
        readBuffer[packetCount] = (uint8_t)(data);
        readBuffer[packetCount + 1] = (uint8_t)(data >> 8);
        packetCount += 2;
    }
    radioSPI.chipDeselect();
}

void sendCmd(SharedSPIDevice<> radioSPI, char* command) {
    while (dataReady.read() != 1) {
    }

    // printf("Command Phase\r\n");
    // wait_ms(mbedPrintWait);

    int length = strlen(command);

    // Concatenate newline if necessary

    char lastC = 0;
    radioSPI.chipSelect();
    for (int i = 0; i < length; i++) {
        char c = command[i];
        if (lastC != 0) {
            uint16_t packet = (c << 8) | lastC;
            //Due to the fact the SPI protocol for this takes turns the return can be ignored
            radioSPI.m_spi->write(packet);
            lastC = 0;
        } else {
            lastC = c;
        }
    }
    radioSPI.chipDeselect();
}

void printReadBuffer() {
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


    char cmdSetSecurity[] = "C3=3\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetSecurity);

    // data phase
    receiveData(radioSPI);


    char cmdSetDHCP[] = "C4=1\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetDHCP);

    // data phase
    receiveData(radioSPI);

    printf("AboutToJoin\r\n");
    wait_ms(mbedPrintWait);

    char cmdJoinNetwork[] = "C0\r\n";
    // command phase
    sendCmd(radioSPI, cmdJoinNetwork);

    printf("commandSent\r\n");
    wait_ms(mbedPrintWait);

    // data phase
    receiveData(radioSPI);


    // if ((int)readBuffer[0] == 0) {
	// 	printf("ERROR: Failed to join network\r\n");
    //     wait_ms(3000);
	// 	return(0);
    // }


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
    printReadBuffer();


    char cmdSetMachineReadable[] = "---\r";
    // command phase
    sendCmd(radioSPI, cmdSetMachineReadable);

    // data phase
    receiveData(radioSPI);


    //TCP client setup
    char cmdSetProtocol[] = "P1=0\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetProtocol);

    // data phase
    receiveData(radioSPI);


    char cmdSetHostIP[] = "P3=192.168.1.108\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetHostIP);

    // data phase
    receiveData(radioSPI);


    char cmdSetPort[] = "P4=25565\r\n";
    // command phase
    sendCmd(radioSPI, cmdSetPort);

    // data phase
    receiveData(radioSPI);


    char cmdStartClient[] = "P6=1\r\n";
    // command phase
    sendCmd(radioSPI, cmdStartClient);

    // data phase
    receiveData(radioSPI);


    timer.reset();
    for (int iter = 0; iter < 10; iter++) {
        timer.start();

        //Transport data
        char cmdSendData[] = "S3=42\r012345678901234567890123456789012345678912";
        // command phase
        sendCmd(radioSPI, cmdSendData);

        // data phase
        receiveData(radioSPI);

        timer.stop();
        printf("Micro Seconds Elapsed: %d\r\n", timer.read_us());
        wait_ms(mbedPrintWait);
        timer.reset();
    }

    printf("Done\r\n");
    return 0;
}
