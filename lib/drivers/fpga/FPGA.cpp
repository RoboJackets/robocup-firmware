#include "FPGA.hpp"

#include <algorithm>

#include "Rtos.hpp"

#include "Logger.hpp"
#include "MacroHelpers.hpp"
#include "software-spi.hpp"

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

FPGA* FPGA::Instance = nullptr;

namespace {
enum {
    CMD_EN_DIS_MTRS = 0x30,
    CMD_R_ENC_W_VEL = 0x80,
    CMD_READ_ENC = 0x91,
    CMD_READ_HALLS = 0x92,
    CMD_READ_DUTY = 0x93,
    CMD_READ_HASH1 = 0x94,
    CMD_READ_HASH2 = 0x95,
    CMD_CHECK_DRV = 0x96,
    CMD_READ_ADC = 0x97
};
}

FPGA::FPGA(std::shared_ptr<SharedSPI> sharedSPI, PinName nCs, PinName initB,
           PinName progB, PinName done)
    : SharedSPIDevice(sharedSPI, nCs, true),
      _initB(initB),
      _done(done),
      _progB(progB, PIN_OUTPUT, OpenDrain, 1) {
    setSPIFrequency(1000000);  // I don't know, it works
}

bool FPGA::configure(const std::string& filepath) {
    // make sure the binary exists before doing anything
    auto fp = fopen(filepath.c_str(), "r");
    if (fp == nullptr) {
        LOG(SEVERE, "No FPGA bitfile!");

        return false;
    }
    fclose(fp);

    // toggle PROG_B to clear out anything prior
    _progB = 0;
    Thread::wait(1);
    _progB = 1;

    // wait for the FPGA to tell us it's ready for the bitstream
    auto fpgaReady = false;
    for (auto i = 0; i < 100; ++i) {
        Thread::wait(10);

        // We're ready to start the configuration process when _initB goes high
        if (_initB) {
            fpgaReady = true;
            break;
        }
    }

    // show INIT_B error if it never went low
    if (!fpgaReady) {
        LOG(SEVERE, "INIT_B pin timed out\t(PRE CONFIGURATION ERROR)");

        return false;
    }

    // Configure the FPGA with the bitstream file, this returns false if file
    // can't be opened
    const auto sendSuccess = send_config(filepath);
    if (sendSuccess) {
        // Wait some extra time in case the _done pin needs time to be asserted
        auto configSuccess = false;
        for (auto i = 0; i < 1000; i++) {
            Thread::wait(1);
            if (_done == true) {
                configSuccess = !_initB;
                break;
            }
        }

        if (configSuccess) {
            // everything worked are we're good to go!
            _isInit = true;
            LOG(DEBUG, "DONE pin state:\t%s", _done ? "HIGH" : "LOW");

            return true;
        }

        LOG(SEVERE, "DONE pin timed out\t(POST CONFIGURATION ERROR)");
    }

    LOG(SEVERE, "FPGA bitstream write error");

    return false;
}

bool FPGA::send_config(const std::string& filepath) {
    constexpr auto bufSize = 50;

    // open the bitstream file
    auto fp = fopen(filepath.c_str(), "r");

    // send it out if successfully opened
    if (fp != nullptr) {
        size_t filesize;
        char buf[bufSize];

        chipSelect();

        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        LOG(DEBUG, "Sending %s (%u bytes) out to the FPGA", filepath.c_str(),
            filesize);

        for (size_t i = 0; i < filesize; i++) {
            bool breakOut = false;
            size_t readSize = fread(buf, 1, bufSize, fp);

            if (!readSize) break;

            for (size_t j = 0; j < bufSize; j++) {
                if (!_initB || _done) {
                    breakOut = true;
                    break;
                }

                m_spi->write(buf[j]);
            }

            if (breakOut) break;
        }

        // SPI dummySPI(RJ_SPI_MOSI, RJ_SPI_MISO, RJ_SPI_SCK);

        chipDeselect();
        fclose(fp);

        return true;
    }

    LOG(SEVERE, "FPGA configuration failed\r\n    Unable to open %s",
        filepath.c_str());

    return false;
}

uint8_t FPGA::read_adc(int16_t* adc_data[12]) {
    uint8_t status;

    chipSelect();
    status = m_spi->write(CMD_READ_ADC);

    for (size_t i = 0; i < 12; i++) {
        uint16_t adc = m_spi->write(0x00) << 8;
        adc |= m_spi->write(0x00);

        uint16_t address = adc;
        if (i % 6 == (address >> 12)) {
            adc =<< 4;

            int16_t value = static_cast<int16_t>(adc);

            adc_data[i] = (value >> 4);
        } else {
            LOG(WARN, "adc_data invalid");

        }

    }


    chipDeselect();

    return status;
}

uint8_t FPGA::read_halls(uint8_t* halls, size_t size) {
    uint8_t status;

    chipSelect();
    status = m_spi->write(CMD_READ_HALLS);

    for (size_t i = 0; i < size; i++) halls[i] = m_spi->write(0x00);

    chipDeselect();

    return status;
}

uint8_t FPGA::read_encs(int16_t* enc_counts, size_t size) {
    uint8_t status;

    chipSelect();
    status = m_spi->write(CMD_READ_ENC);

    for (size_t i = 0; i < size; i++) {
        uint16_t enc = m_spi->write(0x00) << 8;
        enc |= m_spi->write(0x00);
        enc_counts[i] = static_cast<int16_t>(enc);
    }

    chipDeselect();

    return status;
}

uint8_t FPGA::read_duty_cycles(int16_t* duty_cycles, size_t size) {
    uint8_t status;

    chipSelect();
    status = m_spi->write(CMD_READ_DUTY);

    for (size_t i = 0; i < size; i++) {
        uint16_t dc = m_spi->write(0x00) << 8;
        dc |= m_spi->write(0x00);
        duty_cycles[i] = fromSignMag<9>(dc);
    }

    chipDeselect();

    return status;
}

uint8_t FPGA::set_duty_cycles(int16_t* duty_cycles, size_t size) {
    uint8_t status;

    ASSERT(size == 5);
    if (size != 5) {
        LOG(WARN, "set_duty_cycles() requires input buffer to be of size 5");
    }

    // Check for valid duty cycles values
    for (size_t i = 0; i < size; i++)
        if (abs(duty_cycles[i]) > MAX_DUTY_CYCLE) return 0x7F;

    chipSelect();
    status = m_spi->write(CMD_R_ENC_W_VEL);

    for (size_t i = 0; i < size; i++) {
        uint16_t dc = toSignMag<9>(duty_cycles[i]);
        m_spi->write(dc & 0xFF);
        m_spi->write(dc >> 8);
    }

    chipDeselect();

    return status;
}

uint8_t FPGA::set_duty_get_enc(int16_t* duty_cycles, size_t size_dut,
                               int16_t* enc_deltas, size_t size_enc) {
    uint8_t status;

    if (size_dut != 5 || size_enc != 5) {
        LOG(WARN, "set_duty_get_enc() requires input buffers to be of size 5");
    }

    // Check for valid duty cycles values
    for (size_t i = 0; i < size_dut; i++) {
        if (abs(duty_cycles[i]) > MAX_DUTY_CYCLE) return 0x7F;
    }

    chipSelect();
    status = m_spi->write(CMD_R_ENC_W_VEL);

    for (size_t i = 0; i < 5; i++) {
        uint16_t dc = toSignMag<9>(duty_cycles[i]);
        uint16_t enc = m_spi->write(dc & 0xFF) << 8;
        enc |= m_spi->write(dc >> 8);
        enc_deltas[i] = static_cast<int16_t>(enc);
    }

    chipDeselect();

    return status;
}

bool FPGA::git_hash(std::vector<uint8_t>& v) {
    bool dirty_bit;

    chipSelect();
    m_spi->write(CMD_READ_HASH1);

    for (size_t i = 0; i < 10; i++) v.push_back(m_spi->write(0x00));

    chipDeselect();
    chipSelect();

    m_spi->write(CMD_READ_HASH2);

    for (size_t i = 0; i < 11; i++) v.push_back(m_spi->write(0x00));

    chipDeselect();

    // store the dirty bit for returning
    dirty_bit = (v.back() & 0x01);
    // remove the last byte
    v.pop_back();

    // reverse the bytes
    std::reverse(v.begin(), v.end());

    return dirty_bit;
}

void FPGA::gate_drivers(std::vector<uint16_t>& v) {
    chipSelect();

    m_spi->write(CMD_CHECK_DRV);

    // each halfword is structured as follows (MSB -> LSB):
    // | nibble 2: | GVDD_OV  | FAULT    | GVDD_UV  | PVDD_UV  |
    // | nibble 1: | OTSD     | OTW      | FETHA_OC | FETLA_OC |
    // | nibble 0: | FETHB_OC | FETLB_OC | FETHC_OC | FETLC_OC |
    for (size_t i = 0; i < 10; i++) {
        uint16_t tmp = m_spi->write(0x00);
        tmp |= (m_spi->write(0x00) << 8);
        v.push_back(tmp);
    }

    chipDeselect();
}

uint8_t FPGA::motors_en(bool state) {
    uint8_t status;

    chipSelect();
    status = m_spi->write(CMD_EN_DIS_MTRS | (state << 7));
    chipDeselect();

    return status;
}

uint8_t FPGA::watchdog_reset() {
    motors_en(false);
    return motors_en(true);
}

bool FPGA::isReady() { return _isInit; }
