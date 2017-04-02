/**
 * This program sends a robot status packet whenever it receives a control
 * message addressed to it (robot id 1).  It prints a '-->' to the console for
 * every packet sent and a '<--' for every packet received. This program was
 * created to test the cc1201 configuration/driver in order to ensure that
 * everything works and to tune the register settings.  It is meant to be used
 * along with the radio-sender-test program.
 */

#include <cmsis_os.h>
#include <memory>
#include "Mbed.hpp"

#include "CC1201Radio.hpp"
#include "Logger.hpp"
#include "SharedSPI.hpp"
#include "pins-ctrl-2015.hpp"

using namespace std;

const uint8_t ROBOT_UID = 1;

bool initRadio() {
    // setup SPI bus
    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);  // 8 bits per transfer

    // RX/TX leds
    auto rxTimeoutLED = make_shared<FlashingTimeoutLED>(LED1);
    auto txTimeoutLED = make_shared<FlashingTimeoutLED>(LED2);

    // Startup the CommModule interface
    CommModule::Instance = make_shared<CommModule>(rxTimeoutLED, txTimeoutLED);

    // Create a new physical hardware communication link
    globalRadio = std::make_unique<CC1201>(
        sharedSPI, RJ_RADIO_nCS, RJ_RADIO_INT, preferredSettings,
        sizeof(preferredSettings) / sizeof(registerSetting_t));

    return globalRadio->isConnected();
}

void radioRxHandler(RTP::Packet pkt) {
    static int rxCount = 0;
    rxCount++;
    printf("<-- %d\r\n", rxCount);

    RTP::ControlMessage controlMsg;
    bool success = RTP::DeserializeFromBuffer(&controlMsg, pkt.payload.data(),
                                              pkt.payload.size());
    if (!success) {
        printf("bad rx\r\n");
        return;
    }

    if (controlMsg.uid != ROBOT_UID) {
        printf("not addressed by packet\r\n");
        return;
    }

    // send reply packet
    RTP::Packet replyPkt;
    replyPkt.header.port = RTP::Port::CONTROL;
    replyPkt.header.address = RTP::BROADCAST_ADDRESS;

    // create control message and add it to the packet payload
    RTP::RobotStatusMessage msg;
    msg.uid = 1;
    msg.battVoltage = 12;
    RTP::SerializeToVector(msg, &replyPkt.payload);

    // transmit!
    CommModule::Instance->send(std::move(replyPkt));
    static int txCount = 0;
    txCount++;
    printf("--> %d\r\n", txCount);
}

int main() {
    // set baud rate to higher value than the default for faster terminal
    Serial s(RJ_SERIAL_RXTX);
    s.baud(57600);

    // Set the default logging configurations
    isLogging = RJ_LOGGING_EN;
    rjLogLevel = INIT;

    printf("****************************************\r\n");
    LOG(INFO, "Radio test receiver starting...");

    if (initRadio()) {
        LOG(OK, "Radio interface ready on %3.2fMHz!", globalRadio->freq());

        // register handlers for any ports we might use
        for (RTP::Port port :
             {RTP::Port::CONTROL, RTP::Port::PING, RTP::Port::LEGACY}) {
            CommModule::Instance->setRxHandler(&radioRxHandler, port);
            CommModule::Instance->setTxHandler((CommLink*)globalRadio,
                                               &CommLink::sendPacket, port);
        }
    } else {
        LOG(SEVERE, "No radio interface found!");
    }

    DigitalOut radioStatusLed(LED4, globalRadio->isConnected());

    // wait for incoming packets
    while (true) {
        globalRadio->printDebugInfo();
        Thread::wait(1000);
    }
}
