#define __CM3_REV 0x200

// ** DON'T INCLUDE <iostream>! THINGS WILL BREAK! **
#include "Assert.hpp"
#include "BallSensor.hpp"
#include "Commands.hpp"
#include "ConfigStore.hpp"
#include "Decawave.hpp"
#include "FPGA.hpp"
#include "HelperFuncs.hpp"
#include "Logger.hpp"
#include "KickerBoard.hpp"
#include "RadioProtocol.hpp"
#include "RobotDevices.hpp"
#include "RotarySelector.hpp"
#include "Rtos.hpp"
#include "RtosTimerHelper.hpp"
#include "SharedSPI.hpp"
#include "TaskSignals.hpp"
#include "Watchdog.hpp"
#include "io-expander.hpp"
#include "motors.hpp"
#include "neostrip.hpp"
//#include "mpu-6050.hpp"
#include <rc-fshare/robot_model.hpp>

#include <array>
#include <ctime>
#include <string>
#include <configuration/ConfigStore.hpp>
#include <stall/stall.hpp>

// set to 1 to enable CommModule rx/tx stress test
#define COMM_STRESS_TEST (0)

using namespace std;

#ifdef NDEBUG
LocalFileSystem local("local");
#endif

#if COMM_STRESS_TEST
void Task_Simulate_RX_Packet(const void* args) {
    auto commModule = CommModule::Instance;

    while (true) {
        Thread::wait(1);

        rtp::Packet pkt;
        pkt.header.port = rtp::PortType::CONTROL;
        pkt.header.type = rtp::MessageType::CONTROL;
        pkt.header.address = 1;

        rtp::ControlMessage msg;
        msg.uid = 1;
        msg.bodyX = 0;
        msg.bodyY = 0;
        msg.bodyW = 0;
        msg.dribbler = 0;
        msg.kickStrength = 0;
        msg.shootMode = 0;
        msg.triggerMode = 0;
        msg.song = 0;

        auto payload = std::vector<uint8_t>{};
        serializeToVector(msg, &payload);
        pkt.payload = std::move(payload);

        commModule->receive(std::move(pkt));

        Thread::yield();
    }
}
#endif

void Task_Controller(const void* args);
void Task_Controller_UpdateTarget(Eigen::Vector3f targetVel);
void Task_Controller_UpdateDribbler(uint8_t dribbler);
std::array<int16_t, 4> Task_Controller_EncGetClear();
void InitializeCommModule(SharedSPIDevice<>::SpiPtrT sharedSPI);

extern std::array<WheelStallDetection, 4> wheelStallDetection;

/**
 * @brief Sets the hardware configurations for the status LEDs & places
 * into the given state
 *
 * @param[in] state The next state of the LEDs
 */
void statusLights(bool state) {
    DigitalOut init_leds[] = {{RJ_BALL_LED}};
    // the state is inverted because the leds are wired active-low
    for (DigitalOut& led : init_leds) led = !state;
}

/**
 * A function used to convert ascii to a byte for two characters with support
 * for hex values
 * @return value of chars shifted and added with a size of 1 byte
 */
uint8_t atob(char char1, char char2) {
    uint8_t result;
    result = (char1 >= 'a' ? char1 - 'a' + 10 : char1 - '0') << 4;
    if (char2 >= 'a') {
        result += char2 - 'a' + 10;
    } else {
        result += char2 - '0';
    }

    return result;
}

/**
 * The entry point of the system where each submodule's thread is started.
 */
int main() {
    // disable write buffer use during default memory map accesses
    // SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;

    // Store the thread's ID
    const auto mainID = Thread::gettid();
    ASSERT(mainID != nullptr);

    {
        // clear any extraneous rx serial bytes
        Serial s(RJ_SERIAL_RXTX);
        while (s.readable()) s.getc();

        // set baud rate to higher value than the default for faster terminal
        s.baud(57600);
    }

    {
        uint8_t wd_flag = (LPC_WDT->WDMOD >> 2) & 1;
        std::printf("Watchdog caused reset: %s\r\n",
                    (wd_flag > 0) ? "True" : "False");
        wd_flag = (LPC_WDT->WDMOD >> 2) & 1;
    }

    printf("\tCommit Hash:\t%s%s\r\n", git_version_hash,
           git_version_dirty ? " (dirty)" : "");

    // Turn on some startup LEDs to show they're working, they are turned off
    // before we hit the while loop
    statusLights(true);

    // Set the default logging configurations
    isLogging = RJ_LOGGING_EN;
    rjLogLevel = INFO;

    /* Always send out an empty line at startup for keeping the console
     * clean on after a 'reboot' command is called;
     */
    if (isLogging) {
        // reset the console's default settings and enable the cursor
        std::printf("\033[m");
        fflush(stdout);
    }

    // Setup the interrupt priorities before launching each subsystem's task
    // thread.
    setISRPriorities();

    // Force off since the neopixel's hardware is stateless from previous
    // settings
    NeoStrip rgbLED(RJ_NEOPIXEL, 2);
    rgbLED.clear();

    // Set the RGB LEDs to a medium blue while the threads are started up
    auto defaultBrightness = 0.02f;
    rgbLED.brightness(3 * defaultBrightness);
    rgbLED.setPixel(0, NeoColorBlue);
    rgbLED.setPixel(1, NeoColorBlue);
    rgbLED.write();

    // Set pixel 1 to colors corresponding to github hash values
    uint8_t red = atob(git_version_short_hash[0], git_version_short_hash[1]);
    uint8_t green = atob(git_version_short_hash[2], git_version_short_hash[3]);
    uint8_t blue = atob(git_version_short_hash[4], git_version_short_hash[5]);
    rgbLED.setPixel(1, red, green, blue);
    rgbLED.write();

    // Set neopixel 1 to purple if git version is dirty
    if (git_version_dirty) {
        rgbLED.setPixel(1, NeoColorPurple);
        rgbLED.write();
    }

    // Flip off the startup LEDs after a timeout period
    RtosTimerHelper init_leds_off([]() { statusLights(false); }, osTimerOnce);
    init_leds_off.start(RJ_STARTUP_LED_TIMEOUT_MS);

    /// A shared spi bus
    auto spiBus = make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    spiBus->format(8, 0);  // 8 bits per transfer

    // Reprogramming each time (first arg of flash false) is actually
    // faster than checking the full memory to see if we need to reflash.
    KickerBoard::Instance =
        std::make_shared<KickerBoard>(spiBus, RJ_KICKER_nCS, RJ_KICKER_nRESET,
                                      RJ_BALL_LED, "/local/rj-kickr.nib");
    KickerBoard::Instance->flash(false, false);

    KickerBoard::Instance->start();

    init_leds_off.start(RJ_STARTUP_LED_TIMEOUT_MS);

    // Initialize and configure the fpga with the given bitfile
    FPGA::Instance = new FPGA(spiBus, RJ_FPGA_nCS, RJ_FPGA_INIT_B,
                              RJ_FPGA_PROG_B, RJ_FPGA_DONE);

    const auto fpgaInitialized =
        FPGA::Instance->configure("/local/rj-fpga.nib");
    auto fpgaError = false;
    uint8_t fpgaLastStatus = 0;

    if (fpgaInitialized) {
        rgbLED.brightness(3 * defaultBrightness);
        rgbLED.setPixel(0, NeoColorPurple);

        LOG(OK, "FPGA Configuration Successful!");

    } else {
        rgbLED.brightness(4 * defaultBrightness);
        rgbLED.setPixel(0, NeoColorRed);

        LOG(SEVERE, "FPGA Configuration Failed!");
    }
    rgbLED.write();

    // DigitalOut rdy_led(RJ_RDY_LED, !fpgaInitialized);

    // Init IO Expander and turn all LEDs on.  The first parameter to config()
    // sets the first 8 lines to input and the last 8 to output.  The pullup
    // resistors and polarity swap are enabled for the 4 rotary selector lines.
    MCP23017 ioExpander(RJ_I2C_SDA, RJ_I2C_SCL, RJ_IO_EXPANDER_I2C_ADDRESS);
    ioExpander.config(0x00FF, 0x00ff, 0x00ff);
    ioExpander.writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask),
                         IOExpanderErrorLEDMask);

    // DIP Switch 1 controls the radio channel.
    uint8_t currentRadioChannel = 0;
    IOExpanderDigitalInOut radioChannelSwitch(&ioExpander, RJ_DIP_SWITCH_1,
                                              MCP23017::DIR_INPUT);

    // rotary selector for shell id
    RotarySelector<IOExpanderDigitalInOut> rotarySelector(
        {IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT0,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT1,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT2,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT3,
                                MCP23017::DIR_INPUT)});
    // this value is continuously updated in the main loop
    uint8_t robotShellID = rotarySelector.read();

    // Startup the 3 separate threads, being sure that we wait for it
    // to signal back to us that we can startup the next thread. Not doing
    // so results in weird wierd things that are really hard to debug. Even
    // though this is multi-threaded code, that dosen't mean it's
    // a multi-core system.

    // Start the thread task for the on-board control loop
    Thread controller_task(Task_Controller, mainID, osPriorityHigh,
                           DEFAULT_STACK_SIZE / 2);
    Thread::signal_wait(MAIN_TASK_CONTINUE, osWaitForever);

#ifndef NDEBUG
    // Start the thread task for the serial console
    Thread console_task(Task_SerialConsole, mainID, osPriorityBelowNormal);
    Thread::signal_wait(MAIN_TASK_CONTINUE, osWaitForever);
#endif

    // Initialize CommModule and radio
    InitializeCommModule(spiBus);

    // Make sure all of the motors are enabled
    motors_Init();

    // setup analog in on battery sense pin
    // the value is updated in the main loop below
    AnalogIn batt(RJ_BATT_SENSE);
    uint8_t battVoltage = 0;

    // Radio timeout timer
    const auto RadioTimeout = 100;
    RtosTimerHelper radioTimeoutTimer([&]() {
        // Reset radio if no RX packet in specified time
        // globalRadio->reset();
        radioTimeoutTimer.start(RadioTimeout);

        // KickerBoard::Instance->setChargeAllowed(false);
        globalRadio->reset();
        globalRadio->setAddress(rtp::ROBOT_ADDRESS);
    }, osTimerOnce);
    radioTimeoutTimer.start(RadioTimeout);

    // Setup radio protocol handling
    RadioProtocol radioProtocol(CommModule::Instance);
    radioProtocol.setUID(robotShellID);
    radioProtocol.start();

    radioProtocol.debugCallback = [&](const rtp::DebugMessage& msg) {
        //            DebugCommunication::debugResponses = msg.keys;
    };

    radioProtocol.confCallback = [&](const rtp::ConfMessage& msg) {
        for (int i = 0; i < rtp::ConfMessage::length; i++) {
            auto configCommunication = msg.keys[i];
            if (configCommunication != DebugCommunication::ConfigCommunication::
                                           CONFIG_COMMUNICATION_NONE) {
                const auto index = static_cast<int>(configCommunication);
                DebugCommunication::configStore[index] = msg.values[i];
                DebugCommunication::configStoreIsValid[index] = true;
            }
        }
    };

    radioProtocol.rxCallback =
        [&](const rtp::ControlMessage* msg, const bool addressed) {
            // reset timeout
            radioTimeoutTimer.start(RadioTimeout);

            if (addressed) {
                // update target velocity from packet
                Task_Controller_UpdateTarget({
                    static_cast<float>(msg->bodyX) /
                        rtp::ControlMessage::VELOCITY_SCALE_FACTOR,
                    static_cast<float>(msg->bodyY) /
                        rtp::ControlMessage::VELOCITY_SCALE_FACTOR,
                    static_cast<float>(msg->bodyW) /
                        rtp::ControlMessage::VELOCITY_SCALE_FACTOR,
                });

                // dribbler
                Task_Controller_UpdateDribbler(msg->dribbler);

                if (msg->triggerMode == 0) {
                    KickerBoard::Instance->cancelBreakbeam();
                }

                // kick!
                if (msg->shootMode == 0) {
                    uint8_t kickStrength = msg->kickStrength;
                    if (msg->triggerMode == 1) {
                        // kick immediate
                        KickerBoard::Instance->kick(kickStrength);
                    } else if (msg->triggerMode == 2) {
                        // kick on break beam
                        KickerBoard::Instance->kickOnBreakbeam(kickStrength);
                    } else {
                        KickerBoard::Instance->cancelBreakbeam();
                    }
                }
            }
            KickerBoard::Instance->setChargeAllowed(true);

            rtp::RobotStatusMessage reply;
            reply.uid = robotShellID;
            reply.battVoltage = battVoltage;
            reply.ballSenseStatus = KickerBoard::Instance->isBallSensed();

            // report any motor errors
            reply.motorErrors = 0;
            for (auto i = 0; i < 5; i++) {
                auto err = global_motors[i].status.hasError;
                if (err) reply.motorErrors |= (1 << i);
            }

            for (auto i = 0; i < wheelStallDetection.size(); i++) {
                if (wheelStallDetection[i].stalled) {
                    reply.motorErrors |= (1 << i);
                }
            }

            // fpga status
            if (!fpgaInitialized) {
                reply.fpgaStatus = 1;
            } else if (fpgaError) {
                reply.fpgaStatus = 1;
            } else {
                reply.fpgaStatus = 0;  // good
            }

            // kicker status
            reply.kickStatus = KickerBoard::Instance->getVoltage() > 230;
            reply.kickHealthy = KickerBoard::Instance->isHealthy();

            // note: this clears the encoder count
            auto enc_array = Task_Controller_EncGetClear();
            for (auto i = 0; i < enc_array.size(); ++i) {
                reply.encDeltas[i] = enc_array[i];
            }

            //            for (int i=0;
            //            i<rtp::RobotStatusMessage::debug_data_length; i++) {
            //                auto debugType =
            //                DebugCommunication::debugResponses[i];
            //                if (debugType != 0) {
            //                    reply.debug_data[i] =
            //                    DebugCommunication::debugStore[debugType];
            //                } else {
            //                    reply.debug_data[i] =  -1;
            //                }
            //            }

            vector<uint8_t> replyBuf;
            rtp::serializeToVector(reply, &replyBuf);

            return replyBuf;
        };

    // LOG(INIT, "Started charging kicker board.");

    // Set the watdog timer's initial config
    Watchdog::set(RJ_WATCHDOG_TIMER_VALUE);

    // Release each thread into its operations in a structured manner
    controller_task.signal_set(SUB_TASK_CONTINUE);
#ifndef NDEBUG
    console_task.signal_set(SUB_TASK_CONTINUE);
#endif

// #pragma for gcc has bugs in it for selectively disabling warnings
// so we test for NDEBUG instead
#ifndef NDEBUG
    auto tState = osThreadSetPriority(mainID, osPriorityAboveNormal);
    ASSERT(tState == osOK);
#else
    osThreadSetPriority(mainID, osPriorityAboveNormal);
#endif

    auto ll = 0;
    (void)ll;
    uint16_t errorBitmask = 0;
    if (!fpgaInitialized) {
        // assume all motors have errors if FPGA does not work
        errorBitmask |= (1 << RJ_ERR_LED_M1);
        errorBitmask |= (1 << RJ_ERR_LED_M2);
        errorBitmask |= (1 << RJ_ERR_LED_M3);
        errorBitmask |= (1 << RJ_ERR_LED_M4);
        errorBitmask |= (1 << RJ_ERR_LED_DRIB);
    }

// cmd_heapfill();

#if COMM_STRESS_TEST
    Thread sim_task(Task_Simulate_RX_Packet, mainID, osPriorityAboveNormal);
#endif

    while (true) {
        // make sure we can always reach back to main by
        // renewing the watchdog timer periodicly
        Watchdog::renew();

#ifndef NDEBUG
        // periodically reset the console text's format
        ll++;
        if ((ll % 8) == 0) {
            printf("\033[m");
            fflush(stdout);
        }
#endif

        Thread::wait(RJ_WATCHDOG_TIMER_VALUE * 250);

        // Pack errors into bitmask
        errorBitmask &= ~(1 << RJ_ERR_LED_RADIO);
        errorBitmask |= (!globalRadio || !globalRadio->isConnected())
                        << RJ_ERR_LED_RADIO;

        fpgaLastStatus = motors_refresh();
        // top bit of fpga status should be 1 to indicate no errors
        fpgaError = (fpgaLastStatus & (1 << 7)) == 0;

        // add motor errors to bitmask
        static const auto motorErrLedMapping = {
            make_pair(0, RJ_ERR_LED_M1), make_pair(1, RJ_ERR_LED_M2),
            make_pair(2, RJ_ERR_LED_M3), make_pair(3, RJ_ERR_LED_M4),
            make_pair(4, RJ_ERR_LED_DRIB)};

        for (const auto& pair : motorErrLedMapping) {
            const motorErr_t& status = global_motors[pair.first].status;
            // clear the bit
            errorBitmask &= ~(1 << pair.second);
            // set the bit to whatever hasError is set to
            errorBitmask |= (status.hasError << pair.second);
        }

        // get the battery voltage
        battVoltage = (batt.read_u16() >> 8);

        LOG(DEBUG, "Kicker voltage: %u", KickerBoard::Instance->getVoltage());

        // update shell id
        robotShellID = rotarySelector.read();
        radioProtocol.setUID(robotShellID);

        // update radio channel
        auto newRadioChannel = static_cast<uint8_t>(radioChannelSwitch.read());
        if (newRadioChannel != currentRadioChannel) {
            // globalRadio->setChannel(newRadioChannel);
            currentRadioChannel = newRadioChannel;
            LOG(INFO, "Changed radio channel to %u", newRadioChannel);
        }

        // Set error-indicating leds on the control board
        ioExpander.writeMask(~errorBitmask, IOExpanderErrorLEDMask);

        const auto robotHasError =
            errorBitmask || !fpgaInitialized || fpgaError;
        if (robotHasError) {
            // orange - error
            rgbLED.brightness(6 * defaultBrightness);
            rgbLED.setPixel(0, NeoColorOrange);
        } else {
            // no errors, yay!
            rgbLED.brightness(3 * defaultBrightness);
            rgbLED.setPixel(0, NeoColorGreen);
        }
        rgbLED.write();
    }
}

#define _EXTERN extern "C"

_EXTERN void HardFault_Handler() {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, hard_fault_handler_2_const                        \n"
        " bx r2                                                     \n"
        " hard_fault_handler_2_const: .word HARD_FAULT_HANDLER    	\n");
}

_EXTERN void HARD_FAULT_HANDLER(uint32_t* stackAddr) {
    /* These are volatile to try and prevent the compiler/linker optimising them
     * away as the variables never actually get used.  If the debugger won't
     * show the values of the variables, make them global my moving their
     * declaration outside of this function. */
    volatile uint32_t r0{};
    (void)r0;  // disables compiler warning about unused-variables
    volatile uint32_t r1{};
    (void)r1;
    volatile uint32_t r2{};
    (void)r2;
    volatile uint32_t r3{};
    (void)r3;
    volatile uint32_t r12{};
    (void)r12;
    volatile uint32_t lr{};
    (void)lr; /* Link register. */
    volatile uint32_t pc{};
    (void)pc; /* Program counter. */
    volatile uint32_t psr{};
    (void)psr; /* Program status register. */

    r0 = stackAddr[0];
    r1 = stackAddr[1];
    r2 = stackAddr[2];
    r3 = stackAddr[3];
    r12 = stackAddr[4];
    lr = stackAddr[5];
    pc = stackAddr[6];
    psr = stackAddr[7];

    std::printf(
        "\r\n"
        "========== HARD FAULT ==========\r\n"
        "\r\n"
        "  MSP:\t0x%08X\r\n"
        "  HFSR:\t0x%08X\r\n"
        "  CFSR:\t0x%08X\r\n"
        "\r\n"
        "  r0:\t0x%08X\r\n"
        "  r1:\t0x%08X\r\n"
        "  r2:\t0x%08X\r\n"
        "  r3:\t0x%08X\r\n"
        "  r12:\t0x%08X\r\n"
        "  lr:\t0x%08X\r\n"
        "  pc:\t0x%08X\r\n"
        "  psr:\t0x%08X\r\n"
        "\r\n"
        "================================\r\n",
        __get_MSP, SCB->HFSR, SCB->CFSR, r0, r1, r2, r3, r12, lr, pc, psr);

    // do nothing so everything remains unchanged for debugging
    while (true) {
    }
}

_EXTERN void NMI_Handler() { std::printf("NMI Fault!\n"); }

_EXTERN void MemManage_Handler() { std::printf("MemManage Fault!\n"); }

_EXTERN void BusFault_Handler() { std::printf("BusFault Fault!\n"); }

_EXTERN void UsageFault_Handler() { std::printf("UsageFault Fault!\n"); }
