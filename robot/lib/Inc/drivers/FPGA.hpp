#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include <SPI.hpp>
#include <DigitalIn.hpp>
#include <DigitalOut.hpp>

class FPGA { 
public:
    // Global fpga instance.  Must be set to an initialized fpga instance.
    static FPGA* Instance;

    FPGA(std::shared_ptr<SPI> spi_bus, PinName nCs, PinName initB,
         PinName progB, PinName done);

    /// Configure the fpga with the bitfile at the given path
    /// @return true if successful
    bool configure(const std::string& filepath);

    bool isReady();
    uint8_t set_duty_get_enc(int16_t* duty_cycles, size_t size_dut,
                             int16_t* enc_deltas, size_t size_enc);
    uint8_t set_duty_cycles(int16_t* duty_cycles, size_t size);
    uint8_t read_duty_cycles(int16_t* duty_cycles, size_t size);
    uint8_t read_encs(int16_t* enc_counts, size_t size);
    uint8_t read_halls(uint8_t* halls, size_t size);
    uint8_t motors_en(bool state);
    uint8_t watchdog_reset();
    bool git_hash(std::vector<uint8_t>&);
    void gate_drivers(std::vector<uint16_t>&);
    bool send_config(const std::string& filepath);
    void chip_select();
    void chip_deselect();

    static constexpr int FPGA_SPI_FREQ = 1'000'000;
    static const int16_t MAX_DUTY_CYCLE = 511;

private:
    bool _isInit = false;

    std::shared_ptr<SPI> _spi_bus;
    DigitalIn _initB;
    DigitalIn _done;
    DigitalOut _progB;
};
