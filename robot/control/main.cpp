#include <mtrain.hpp>
#include <SPI.hpp>

#include <array>
#include <memory>
#include <vector>

#include "FPGA.hpp"

#include "iodefs.h"
//#include "motors.hpp"

std::shared_ptr<SPI> _fpga_kicker_spi_bus;
std::shared_ptr<SPI> _radio_spi_bus;
std::shared_ptr<SPI> _dot_star_spi_bus;

std::shared_ptr<FPGA> _fpga;

void init();
void loop();

int main() {
	init();

	while (true) {
		loop();
	}
}

void init() {
	// initialize the SPI busses
	_fpga_kicker_spi_bus = std::make_shared<SPI>(FPGA_KICKER_SPI_BUS);
	_radio_spi_bus = std::make_shared<SPI>(RADIO_SPI_BUS);
	_dot_star_spi_bus = std::make_shared<SPI>(DOT_STAR_SPI_BUS);

	// create and program the FPGA
	_fpga = std::make_shared<FPGA>(_fpga_kicker_spi_bus, FPGA_CS, FPGA_INIT, FPGA_DONE, FPGA_PROG);
	FPGA::Instance = _fpga;
	FPGA::Instance->configure();

	//motors_Init();
}

void loop() {
	std::array<int16_t, 5> duty_cycles;
	duty_cycles[0] = 0x30;
	uint8_t sb = FPGA::Instance->read_duty_cycles(duty_cycles.data(), duty_cycles.size());
	if (sb != 0x7F) {
		FPGA::Instance->set_duty_cycles(duty_cycles.data(), duty_cycles.size());	
	}
}

