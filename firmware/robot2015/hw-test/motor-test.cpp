#include <mbed.h>

#include "Logger.hpp"

#include "FPGA.hpp"
#include "RobotDevices.hpp"
#include "RotarySelector.hpp"
#include "io-expander.hpp"

#define RJ_ENABLE_ROBOT_CONSOLE

#define MOTOR_MAX_SAMPLES 2
// Ticker lifeLight;
// DigitalOut ledOne(LED1);
// DigitalOut ledTwo(LED2);

LocalFileSystem fs("local");

/**
 * timer interrupt based light flicker
 */
// void imAlive() { ledOne = !ledOne; }
typedef uint16_t motorVel_t;
struct motorErr_t {
    /*
    Available Flags:
    GVDD_OV, GVDD_UV, PVDD_UV
    OTSD, OTW
    FETHA_OC, FETLA_OC
    FETHB_OC, FETLB_OC
    FETHC_OC, FETLC_OC
    */
    bool hasError;
    std::array<uint16_t, 2> drvStatus;
};
struct motor_t {
    motorVel_t vel;
    uint16_t hall;
    std::array<uint32_t, MOTOR_MAX_SAMPLES> enc;
    motorErr_t status;
    std::string desc;
};

motor_t mtrEx = {.vel = 0x4D,
                 .hall = 0x0A,
                 .enc = {0x23, 0x18},
                 .status = {.hasError = false, .drvStatus = {0x26, 0x0F}},
                 .desc = "Motor"};

const int NUM_MOTORS = 5;
std::vector<motor_t> global_motors(NUM_MOTORS, mtrEx);

void motors_show() {
    std::array<int16_t, NUM_MOTORS> duty_cycles = {0};
    std::array<uint8_t, NUM_MOTORS> halls = {0};
    std::array<int16_t, NUM_MOTORS> enc_deltas = {0};

    FPGA::Instance->read_duty_cycles(duty_cycles.data(), duty_cycles.size());
    FPGA::Instance->read_halls(halls.data(), halls.size());
    FPGA::Instance->read_encs(enc_deltas.data(), enc_deltas.size());

    // get the driver register values from the fpga
    std::vector<uint16_t> driver_regs;
    FPGA::Instance->gate_drivers(driver_regs);

    // The status byte fields:
    //   { sys_rdy, watchdog_trigger, motors_en, is_connected[4:0] }
    uint8_t status_byte = FPGA::Instance->watchdog_reset();

    printf("\033[?25l\033[25mStatus:\033[K\t\t\t%s\033E",
           status_byte & 0x20 ? "ENABLED" : "DISABLED");
    printf(
        "\033[KLast Update:\t\t%-6.2fms\t%s\033E",
        (static_cast<float>(enc_deltas.back()) * (1 / 18.432) * 2 * 64) / 1000,
        status_byte & 0x40 ? "[EXPIRED]" : "[OK]     ");
    printf("\033[K    ID\t\tVEL\tHALL\tENC\tDIR\tSTATUS\t\tFAULTS\033E");
    for (size_t i = 0; i < duty_cycles.size() - 1; i++) {
        printf("\033[K    %s\t%-3d\t%-3u\t%-5d\t%s\t%s\t0x%03X\033E",
               global_motors[i].desc.c_str(), duty_cycles[i], halls[i],
               enc_deltas[i], duty_cycles[i] < 0 ? "CW" : "CCW",
               (status_byte & (1 << i)) ? "[OK]    " : "[UNCONN]",
               driver_regs[i]);
    }
    printf("\033[K    %s\t%-3u\t%-3u\tN/A\t%s\t%s\t0x%03X\033E",
           global_motors.back().desc.c_str(), duty_cycles.back() & 0x1FF,
           halls.back(), duty_cycles.back() < 0 ? "CW" : "CCW",
           (status_byte & (1 << (enc_deltas.size() - 1))) ? "[OK]    "
                                                          : "[UNCONN]",
           driver_regs.back());
}

int main() {
    Serial s(RJ_SERIAL_RXTX);
    while (s.readable()) s.getc();
    s.baud(57600);

    isLogging = RJ_LOGGING_EN;
    rjLogLevel = INFO;

    if (isLogging) {
        // reset the console's default settings and enable the cursor
        printf("\033[m");
        fflush(stdout);
    }

    // lifeLight.attach(&imAlive, 0.25);

    // A shared spi bus used for the fpga
    shared_ptr<SharedSPI> sharedSPI =
        make_shared<SharedSPI>(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    sharedSPI->format(8, 0);  // 8 bits per transfer

    // Initialize and configure the fpga with the given bitfile
    FPGA::Instance = new FPGA(sharedSPI, RJ_FPGA_nCS, RJ_FPGA_INIT_B,
                              RJ_FPGA_PROG_B, RJ_FPGA_DONE);
    bool fpgaReady = FPGA::Instance->configure("/local/rj-fpga.nib");

    if (fpgaReady) {
        LOG(INIT, "FPGA Configuration Successful!");

    } else {
        LOG(FATAL, "FPGA Configuration Failed!");
    }

    // DigitalOut rdy_led(RJ_RDY_LED, !fpgaReady);

    // Init IO Expander and turn all LEDs on.  The first parameter to config()
    // sets the first 8 lines to input and the last 8 to output.  The pullup
    // resistors and polarity swap are enabled for the 4 rotary selector lines.
    MCP23017 ioExpander(RJ_I2C_SDA, RJ_I2C_SCL, RJ_IO_EXPANDER_I2C_ADDRESS);
    ioExpander.config(0x00FF, 0x00f0, 0x00f0);
    ioExpander.writeMask(static_cast<uint16_t>(~IOExpanderErrorLEDMask),
                         IOExpanderErrorLEDMask);

    // rotary selector for setting motor velocities without needing a computer
    RotarySelector<IOExpanderDigitalInOut> rotarySelector(
        {IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT0,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT1,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT2,
                                MCP23017::DIR_INPUT),
         IOExpanderDigitalInOut(&ioExpander, RJ_HEX_SWITCH_BIT3,
                                MCP23017::DIR_INPUT)});

    uint16_t duty_cycle_all = 0;

    std::vector<int16_t> duty_cycles;
    duty_cycles = {0, 0, 0, 0, 0};
    // duty_cycles.assign(5, 0);

    while (true) {
        std::vector<int16_t> enc_deltas(5);
        FPGA::Instance->set_duty_get_enc(duty_cycles.data(), duty_cycles.size(),
                                         enc_deltas.data(),
                                         enc_deltas.capacity());

        // get a reading from the rotary selector
        const uint8_t rotary_vel = rotarySelector.read();

        // fixup the duty cycle to be centered around 0 and
        // increasing from 0 for both CW & CCW spins of the
        // rotary selector
        const uint8_t duty_cycle_multiplier =
            0x07 &
            static_cast<uint8_t>(8 - abs(8 - static_cast<int>(rotary_vel)));

        // calculate a duty cycle in steps of 73, this means max is 73 * 7 = 511
        duty_cycle_all = duty_cycle_multiplier * 73;

        // set the direction, the bit shifting should be self explanatory here
        // (that was a joke guys...calm down)
        duty_cycle_all |= (((rotary_vel & (1 << 3)) >> 3) << 9);

        // limit max value
        duty_cycle_all =
            100;  // std::min(duty_cycle_all, static_cast<uint16_t>(511));

        // set the duty cycle values all to our determined value according to
        // the rotary selector
        std::fill(duty_cycles.begin(), duty_cycles.end(), duty_cycle_all);
        motors_show();

        // move cursor back 8 rows
        printf("\033[%uA", 8);
        // Console::Instance()->Flush();
        wait_ms(3);
    }
}
