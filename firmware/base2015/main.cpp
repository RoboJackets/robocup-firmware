#include <cmsis_os.h>
#include <memory>
#include "Mbed.hpp"

#include "CC1201Radio.hpp"
#include "Decawave.hpp"
#include "Logger.hpp"
#include "Logger.hpp"
#include "RJBaseUSBDevice.hpp"
#include "SharedSPI.hpp"
#include "Watchdog.hpp"
#include "firmware-common/base2015/usb-interface.hpp"
#include "pins.hpp"

#define RJ_WATCHDOG_TIMER_VALUE 2  // seconds

using namespace std;

// setup USB interface with custom vendor/product ids
RJBaseUSBDevice usbLink(RJ_BASE2015_VENDOR_ID, RJ_BASE2015_PRODUCT_ID,
                        RJ_BASE2015_RELEASE);

bool initRadio() {
    // setup SPI bus
    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);  // 8 bits per transfer

    // RX/TX leds
    auto rxTimeoutLED = make_shared<FlashingTimeoutLED>(LED1);
    auto txTimeoutLED = make_shared<FlashingTimeoutLED>(LED2);

    // Startup the CommModule interface
    CommModule::Instance = make_shared<CommModule>();

    // Construct an object pointer for the radio
    globalRadio =
        std::make_unique<Decawave>(sharedSPI, RJ_RADIO_nCS, RJ_RADIO_INT, RJ_RADIO_nRESET);

    return globalRadio->isConnected();
}

void radioRxHandler(rtp::Packet pkt) {
    LOG(DEBUG, "radioRxHandler()");
    // write packet content (including header) out to EPBULK_IN
    vector<uint8_t> buf;
    pkt.pack(&buf);

    // drop the packet if it's the wrong size. Thsi will need to be changed if
    // we have variable-sized reply packets
    if (buf.size() != rtp::ReverseSize) {
        LOG(WARN, "Dropping packet, wrong size '%u', should be '%u'",
            buf.size(), rtp::ReverseSize);
        return;
    }

    bool success = usbLink.writeNB(EPBULK_IN, buf.data(), buf.size(),
                                   MAX_PACKET_SIZE_EPBULK);
    (void)success;  // disable unused-variable warnings

    // TODO(justin): add this message back in. For some reason, the usb system
    // reports failure *unless* I add a print statement inside
    // USBDevice.writeNB() after result = endpointWrite().  No idea why this is
    // the case
    //
    // if (!success) LOG(WARN, "Failed to transfer received %u byte packet over
    // usb", pkt.payload.size());
}

int main() {
    // set baud rate to higher value than the default for faster terminal
    Serial s(RJ_SERIAL_RXTX);
    s.baud(57600);

    // Set the default logging configurations
    isLogging = RJ_LOGGING_EN;
    rjLogLevel = INFO;

    printf("****************************************\r\n");
    LOG(INFO, "Base station starting...");

    if (initRadio()) {
        // LOG(OK, "Radio interface ready on %3.2fMHz!",
        // globalRadio->freq());

        // register handlers for any ports we might use
        for (rtp::PortType port : {rtp::PortType::CONTROL, rtp::PortType::PING,
                                   rtp::PortType::LEGACY}) {
            CommModule::Instance->setRxHandler(&radioRxHandler, port);
            CommModule::Instance->setTxHandler(
                dynamic_cast<CommLink*>(globalRadio.get()),
                &CommLink::sendPacket, port);
        }
    } else {
        LOG(SEVERE, "No radio interface found!");
    }

    globalRadio->setAddress(rtp::BASE_STATION_ADDRESS);

    DigitalOut radioStatusLed(LED4, globalRadio->isConnected());

    // set callbacks for usb control transfers
    usbLink.writeRegisterCallback =
        [](uint8_t reg, uint8_t val) {  // globalRadio->writeReg(reg, val);
            LOG(DEBUG, "Trying to write");
        };
    usbLink.readRegisterCallback =
        [](uint8_t reg) {  // return globalRadio->readReg(reg);
            LOG(DEBUG, "Tring to read");
            return 0;
        };
    usbLink.strobeCallback =
        [](uint8_t strobe) {  // globalRadio->strobe(strobe);
            LOG(DEBUG, "trying to strobe");
        };
    usbLink.setRadioChannelCallback = [](uint8_t chanNumber) {
        // globalRadio->setChannel(chanNumber);
        LOG(OK, "(Not) Set radio channel to %u", chanNumber);
    };

    LOG(OK, "Initializing USB interface...");
    usbLink.connect();  // note: this blocks until the link is connected
    LOG(OK, "Initialized USB interface!");

    // Set the watdog timer's initial config
    Watchdog::set(RJ_WATCHDOG_TIMER_VALUE);

    LOG(OK, "Listening for commands over USB");

    // buffer to read data from usb bulk transfers into
    auto buf = std::array<uint8_t, MAX_PACKET_SIZE_EPBULK>{};
    auto bufSize = uint32_t{};

    while (true) {
        // make sure we can always reach back to main by renewing the watchdog
        // timer periodically
        Watchdog::renew();
        // attempt to read data from EPBULK_OUT
        // if data is available, write it into @pkt and send it
        if (usbLink.readEP_NB(EPBULK_OUT, buf.data(), &bufSize,
                              MAX_PACKET_SIZE_EPBULK)) {
            LOG(DEBUG, "Read %d bytes from BULK IN", bufSize);

            // construct packet from buffer received over USB
            rtp::Packet pkt(buf);

            // send to all robots
            pkt.header.address = rtp::ROBOT_ADDRESS;

            // transmit!
            CommModule::Instance->send(std::move(pkt));
        }
        Thread::yield();
    }
}
