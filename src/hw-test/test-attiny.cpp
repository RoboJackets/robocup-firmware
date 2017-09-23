#include "Mbed.hpp"
#include "KickerBoard.hpp"
#include "pins-ctrl-2015.hpp"

Ticker lifeLight;
DigitalOut ledOne(LED1);
DigitalOut ledTwo(LED2);

LocalFileSystem fs("local");

/**
 * timer interrupt based light flicker
 */
void imAlive() { ledOne = !ledOne; }

/**
 * system entry point
 */
int main() {
    lifeLight.attach(&imAlive, 0.25);

    /// A shared spi bus
    auto spiBus = make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    spiBus->format(8, 0);  // 8 bits per transfer

    //  initialize kicker board and flash it with new firmware if necessary
    KickerBoard kickerBoard(spiBus, RJ_KICKER_nCS, RJ_KICKER_nRESET,
                            RJ_BALL_LED, "/local/rj-kickr.nib");
    bool kickerSuccess = !kickerBoard.flash(true, true);

    // Set LED to indicate kicker success
    ledTwo = kickerSuccess;
}
