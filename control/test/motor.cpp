#include "main.hpp"

bool fake_radio_flag;

int motor::main() {
	fake_radio_flag = true;
	main::main();
}
