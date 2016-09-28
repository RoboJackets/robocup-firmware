#include "KickerBoard.hpp"
#include "SharedSPI.hpp"
#include "mbed.h"
#include "pins-ctrl-2015.hpp"

LocalFileSystem fs("local");

Ticker lifeLight;
DigitalOut ledOne(LED1);
DigitalOut ledTwo(LED2);

// DigitalOut cs(p8);
Serial pc(USBTX, USBRX);  // tx and rx
                          // DigitalOut n_kick(p9);

/**
 * timer interrupt based light flicker
 */
void imAlive() { ledOne = !ledOne; }

int main() {
    lifeLight.attach(&imAlive, 0.25);

    pc.baud(57600);  // set up the serial
    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);
    KickerBoard kickerBoard(sharedSPI, RJ_KICKER_nCS, RJ_KICKER_nRESET,
                            "/local/rj-kickr.nib");  // nCs, nReset
    bool kickerReady = kickerBoard.flash(false, true);
    printf("Flashed kicker, success = %s\r\n", kickerReady ? "TRUE" : "FALSE");

    char getCmd;

    while (true) {
        ledTwo = !ledTwo;
        Thread::wait(10);
        if (pc.readable()) {
            getCmd = pc.getc();

            pc.printf("%c: ", getCmd);
            switch (getCmd) {
                case 'k':
                    pc.printf("Kicked, Resp: 0x%02X", kickerBoard.kick(240));
                    break;
                case 'c':
                    pc.printf("Chipped, Resp: 0x%02X", kickerBoard.chip(240));
                    break;
                case 'r':
                    pc.printf("Read Volts: %d", kickerBoard.read_voltage());
                    break;
                case 'h':
                    pc.printf("Set charging, Resp: 0x%02X",
                              kickerBoard.charge());
                    break;
                case 'j':
                    pc.printf("Stop charging, Resp: 0x%02X",
                              kickerBoard.stop_charging());
                    break;
                case 'p':
                    pc.printf("Pinged, Resp: 0x%02X",
                              kickerBoard.is_pingable());
                    break;
                case '1':
                    pc.printf("Kick Resp: 0x%02X",
                              kickerBoard.is_kick_debug_pressed());
                    break;
                case '2':
                    pc.printf("Chip Resp: 0x%02X",
                              kickerBoard.is_chip_debug_pressed());
                    break;
                case '3':
                    pc.printf("Charge Resp: 0x%02X",
                              kickerBoard.is_charge_debug_pressed());
                    break;
                default:
                    pc.printf("Invalid command");
                    break;
            }

            pc.printf("\r\n");
            fflush(stdout);

            Thread::wait(2);
        }
    }
}
