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
std::shared_ptr<std::array<int16_t, 5>> _duty_cycles;

bool init();
void loop();

int main() {
	DigitalOut led1(LED1);
	DigitalOut led2(LED2);
	DigitalOut led4(LED4);
	
	led1 = 1;

	bool r = init();
	if (r) {
		led2 = 1;
	}

	led4 = 1;
	while (true) {
		//loop();
		HAL_Delay(100);
		led4.toggle();
	}
}

bool init() {
	// initialize the SPI busses
	_fpga_kicker_spi_bus = std::make_shared<SPI>(FPGA_KICKER_SPI_BUS);
	_radio_spi_bus = std::make_shared<SPI>(RADIO_SPI_BUS);
	_dot_star_spi_bus = std::make_shared<SPI>(DOT_STAR_SPI_BUS);

	_duty_cycles = std::make_shared<std::array<int16_t, 5>>();

	// create and program the FPGA
	_fpga = std::make_shared<FPGA>(_fpga_kicker_spi_bus, FPGA_CS, FPGA_INIT, FPGA_DONE, FPGA_PROG);
	FPGA::Instance = _fpga;
	bool res = FPGA::Instance->configure();
	if (res) {
		return true;
	}

	return false;
}

void loop() {
	//uint8_t sb = FPGA::Instance->read_duty_cycles(_duty_cycles->data(), _duty_cycles->size());
	//if (sb != 0x7F) {
		for (int i = 0; i < 5; i++) {
			_duty_cycles->at(i) = 0x7F;
		}
		FPGA::Instance->set_duty_cycles(_duty_cycles->data(), _duty_cycles->size());	
	//}
}

