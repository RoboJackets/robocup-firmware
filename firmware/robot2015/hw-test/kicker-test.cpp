#include <mbed.h>
#include <rtos.h>

#include "KickerBoard.hpp"
#include "SharedSPI.hpp"
#include "pins-ctrl-2015.hpp"

using namespace std;

const int BAUD_RATE = 57600;
const float ALIVE_BLINK_RATE = 0.25;

LocalFileSystem fs("local");

Ticker lifeLight;
DigitalOut ledOne(LED1);
DigitalOut ledTwo(LED2);

Serial pc(USBTX, USBRX);  // tx and rx

/**
 * timer interrupt based light flicker
 */
void imAlive() { ledOne = !ledOne; }

std::string bool_to_string(bool b) { return b ? "true" : "false"; }

int main() {
    lifeLight.attach(&imAlive, ALIVE_BLINK_RATE);

    pc.baud(BAUD_RATE);  // set up the serial
    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);
    KickerBoard kicker(sharedSPI, RJ_KICKER_nCS, RJ_KICKER_nRESET,
                       "/local/rj-kickr.nib");  // nCs, nReset
    bool kickerReady = kicker.flash(false, true);
    printf("Flashed kicker, success = %s\r\n", kickerReady ? "TRUE" : "FALSE");

    char getCmd;

    string response;
    bool success = false;
    while (true) {
        ledTwo = !ledTwo;
        Thread::wait(10);
        if (pc.readable()) {
            getCmd = pc.getc();

            pc.printf("%c: ", getCmd);
            switch (getCmd) {
                case 'k':
                    response = "Kicked";
                    success = kicker.kick(DB_KICK_TIME);
                    break;
                case 'c':
                    response = "Chipped";
                    success = kicker.chip(DB_CHIP_TIME);
                    break;
                case 'r':
                    response = "Read Voltage";
                    uint8_t volts;
                    success = kicker.read_voltage(volts);
                    response += ", voltage: " + to_string(volts);
                    break;
                case 'h':
                    response = "Set charging";
                    success = kicker.charge();
                    break;
                case 'j':
                    response = "Stop charging";
                    success = kicker.stop_charging();
                    break;
                case 'p':
                    response = "Pinged";
                    success = kicker.is_pingable();
                    break;
                default:
                    response = "Invalid command";
                    break;
            }

            response += ", success: " + bool_to_string(success);
            pc.printf(response.c_str());
            pc.printf("\r\n");
            fflush(stdout);

            Thread::wait(2);
        }
    }
}
