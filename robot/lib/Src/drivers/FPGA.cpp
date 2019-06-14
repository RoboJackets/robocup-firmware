#include "drivers/FPGA.hpp"

#include <memory>
#include <stdint.h>

#include "device-bins/fpga_bin.h"

template <size_t SIGN_INDEX>
uint16_t toSignMag(int16_t val) {
    return static_cast<uint16_t>((val < 0) ? ((-val) | 1 << SIGN_INDEX) : val);
}

template <size_t SIGN_INDEX>
int16_t fromSignMag(uint16_t val) {
    if (val & 1 << SIGN_INDEX) {
        val ^= 1 << SIGN_INDEX;  // unset sign bit
        val *= -1;               // negate
    }
    return val;
}

/**
 * This command set represents the commands that can
 * be issued to the FPGA.
 *
 * This command set should be synchronized with the
 * Verilog Enum in the FPGA firmware.
 */
namespace {
enum {
    CMD_EN_DIS_MTRS = 0x30,
    CMD_R_ENC_W_VEL = 0x80,
    CMD_READ_ENC = 0x91,
    CMD_READ_HALLS = 0x92,
    CMD_READ_DUTY = 0x93,
    CMD_READ_HASH1 = 0x94,
    CMD_READ_HASH2 = 0x95,
    CMD_CHECK_DRV = 0x96
};
}

/**
 * Initializes the FPGA instance.
 *
 * @param spi_bus, the SPI spi_bus the FPGA is connected to
 * @param nCs, the PinName for the negated chip select line
 * @param initB, the PinName for the initB line from the FPGA
 * @param progB, the PinName for the progB command line to the FPGA
 * @param done, the PinName for the done line from the FPGA confirming
 *              successful flash
 *
 * initialies FPGA::Instance.
 */
FPGA::FPGA(std::shared_ptr<SPI> spi_bus, PinName nCs, PinName initB,
           PinName progB, PinName done)
    : _spi_bus(spi_bus),
      _nCs(nCs, PullType::PullNone, PinMode::PushPull, PinSpeed::Low, true),
      _initB(initB),
      _done(done),
      _progB(progB, PullType::PullNone, PinMode::OpenDrain, PinSpeed::Low, false) {
    _spi_bus->frequency(FPGA_SPI_FREQ);
    _progB = 1;
}

bool FPGA::configure() {
    // toggle PROG_B to clear out anything prior
    _progB = 0;
    HAL_Delay(1);
    _progB = 1;

    // wait for the FPGA to tell us it's ready for the bitstream
    auto fpgaReady = false;
    for (auto i = 0; i < 100; ++i) {
        HAL_Delay(10);

        // We're ready to start the configuration process when _initB goes high
        if (_initB) {
            fpgaReady = true;
            break;
        }
    }

    // show INIT_B error if it never went low
    if (!fpgaReady) {
        //LOG(SEVERE, "INIT_B pin timed out\t(PRE CONFIGURATION ERROR)");

        return false;
    }



    // Configure the FPGA with the bitstream file, this returns false if file
    // can't be opened
    const auto sendSuccess = send_config();
    if (sendSuccess) {
        // Wait some extra time in case the _done pin needs time to be asserted
        auto configSuccess = false;
        for (auto i = 0; i < 1000; i++) {
            HAL_Delay(1);
            if (_done == true) {
                configSuccess = _initB;
                break;
            }
        }

        if (configSuccess) {
            // everything worked are we're good to go!
            _isInit = true;
            //LOG(DEBUG, "DONE pin state:\t%s", _done ? "HIGH" : "LOW");

            return true;
        }

        //LOG(SEVERE, "DONE pin timed out\t(POST CONFIGURATION ERROR)");
    }

    //LOG(SEVERE, "FPGA bitstream write error");

    return false;
}

bool FPGA::send_config() {
    chip_select();
    
    for (size_t i = 0; i < FPGA_BYTES_LEN; i++) {
	if (_done) {
		break;
	}
        
        _spi_bus->transmit(FPGA_BYTES[i]);
    }
    
    // SPI dummySPI(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);
    
    chip_deselect();
    
    return true;
}

uint8_t FPGA::read_halls(uint8_t* halls, size_t size) {
    uint8_t status;

    chip_select();
    status = _spi_bus->transmitReceive(CMD_READ_HALLS);

    for (size_t i = 0; i < size; i++) halls[i] = _spi_bus->transmitReceive(0x00);

    chip_deselect();

    return status;
}

uint8_t FPGA::read_encs(int16_t* enc_counts, size_t size) {
    uint8_t status;

    chip_select();
    status = _spi_bus->transmitReceive(CMD_READ_ENC);

    for (size_t i = 0; i < size; i++) {
        uint16_t enc = _spi_bus->transmitReceive(0x00) << 8;
        enc |= _spi_bus->transmitReceive(0x00);
        enc_counts[i] = static_cast<int16_t>(enc);
    }

    chip_deselect();

    return status;
}

uint8_t FPGA::read_duty_cycles(int16_t* duty_cycles, size_t size) {
    uint8_t status;

    chip_select();
    status = _spi_bus->transmitReceive(CMD_READ_DUTY);

    for (size_t i = 0; i < size; i++) {
        uint16_t dc = _spi_bus->transmitReceive(0x00) << 8;
        dc |= _spi_bus->transmitReceive(0x00);
        duty_cycles[i] = fromSignMag<9>(dc);
    }

    chip_deselect();

    return status;
}

uint8_t FPGA::set_duty_cycles(int16_t* duty_cycles, size_t size) {
    uint8_t status;

    //ASSERT(size == 5);
    //if (size != 5) {
        //LOG(WARN, "set_duty_cycles() requires input buffer to be of size 5");
    //}

    // Check for valid duty cycles values
    for (size_t i = 0; i < size; i++)
        if (abs(duty_cycles[i]) > MAX_DUTY_CYCLE) return 0x7F;

    chip_select();
    status = _spi_bus->transmitReceive(CMD_R_ENC_W_VEL);


    for (size_t i = 0; i < size; i++) {
        uint16_t dc = toSignMag<9>(duty_cycles[i]);
        _spi_bus->transmit(dc & 0xFF);
        _spi_bus->transmit(dc >> 8);
    }

    chip_deselect();

    return status;
}

uint8_t FPGA::set_duty_get_enc(int16_t* duty_cycles, size_t size_dut,
                               int16_t* enc_deltas, size_t size_enc) {
    _spi_bus->frequency(15'000'000);
    uint8_t status;

    if (size_dut != 5 || size_enc != 5) {
        //LOG(WARN, "set_duty_get_enc() requires input buffers to be of size 5");
    }

    // Check for valid duty cycles values
    for (size_t i = 0; i < size_dut; i++) {
        if (abs(duty_cycles[i]) > MAX_DUTY_CYCLE) return 0x7F;
    }

    chip_select();
    status = _spi_bus->transmitReceive(CMD_R_ENC_W_VEL);

    for (size_t i = 0; i < 5; i++) {
        uint16_t dc = toSignMag<9>(duty_cycles[i]);
        uint16_t enc = _spi_bus->transmitReceive(dc & 0xFF) << 8;
        enc |= _spi_bus->transmitReceive(dc >> 8);
        enc_deltas[i] = static_cast<int16_t>(enc);
    }

    chip_deselect();

    return status;
}

bool FPGA::git_hash(std::vector<uint8_t>& v) {
    bool dirty_bit;

    chip_select();
    _spi_bus->transmit(CMD_READ_HASH1);

    for (size_t i = 0; i < 10; i++) v.push_back(_spi_bus->transmitReceive(0x00));

    chip_deselect();
    chip_select();

    _spi_bus->transmit(CMD_READ_HASH2);

    for (size_t i = 0; i < 11; i++) v.push_back(_spi_bus->transmitReceive(0x00));

    chip_deselect();

    // store the dirty bit for returning
    dirty_bit = (v.back() & 0x01);
    // remove the last byte
    v.pop_back();

    // reverse the bytes
    std::reverse(v.begin(), v.end());

    return dirty_bit;
}

void FPGA::gate_drivers(std::vector<uint16_t>& v) {
    chip_select();

    _spi_bus->transmit(CMD_CHECK_DRV);

    // each halfword is structured as follows (MSB -> LSB):
    // | nibble 2: | GVDD_OV  | FAULT    | GVDD_UV  | PVDD_UV  |
    // | nibble 1: | OTSD     | OTW      | FETHA_OC | FETLA_OC |
    // | nibble 0: | FETHB_OC | FETLB_OC | FETHC_OC | FETLC_OC |
    for (size_t i = 0; i < 10; i++) {
        uint16_t tmp = _spi_bus->transmitReceive(0x00);
        tmp |= (_spi_bus->transmitReceive(0x00) << 8);
        v.push_back(tmp);
    }

    chip_deselect();
}

uint8_t FPGA::motors_en(bool state) {
    uint8_t status;

    chip_select();
    status = _spi_bus->transmitReceive(CMD_EN_DIS_MTRS | (state << 7));
    chip_deselect();

    return status;
}

uint8_t FPGA::watchdog_reset() {
    motors_en(false);
    return motors_en(true);
}

void FPGA::chip_select() {
    _spi_bus->frequency(FPGA_SPI_FREQ);
    _nCs = 1;
}

void FPGA::chip_deselect() {
    _nCs = 0;
}

bool FPGA::isReady() { return _isInit; }

