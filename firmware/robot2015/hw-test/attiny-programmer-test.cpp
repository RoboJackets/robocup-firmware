#include <mbed.h>
#include <rtos.h>
#include <logger.hpp>

#include "AVR910.hpp"
#include "SharedSPI.hpp"
#include "pins-ctrl-2015.hpp"

const int BAUD_RATE = 57600;
const float ALIVE_BLINK_RATE = 0.25;

LocalFileSystem fs("local");
Serial pc(USBTX, USBRX);  // tx and rx
Ticker lifeLight;
DigitalOut ledOne(LED1);

std::string filename("/local/rj.nib");

/**
 * timer interrupt based light flicker
 */
void imAlive() { ledOne = !ledOne; }

int main() {
    isLogging = RJ_LOGGIN_EN;
    rjLogLevel = INF2;

    lifeLight.attach(&imAlive, ALIVE_BLINK_RATE);
    pc.baud(BAUD_RATE);

    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);
    FILE* fp = fopen(filename.c_str(), "r");
    AVR910 programmer(sharedSPI, RJ_KICKER_nCS, RJ_KICKER_nRESET);

    if (fp == nullptr) {
        pc.printf("Failed to open file.\r\n");
        return -1;
    }

    pc.printf("Attempting to program...\r\n");
    bool nSuccess =
        programmer.program(fp, ATTINY84A_PAGESIZE, ATTINY84A_NUM_PAGES);

    if (nSuccess) {
        pc.printf("Programming failed.\r\n");
    } else {
        pc.printf("Programming succeeded.\r\n");
    }

    fclose(fp);
    lifeLight.detach();
    return 0;
}
