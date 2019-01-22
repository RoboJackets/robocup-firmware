#include "mtrain.h"

void digitalin_init(pin_name p);
void digitalin_init_ex(pin_name p, pull_type pull);
void digitalin_deinit(pin_name p);

int digitalin_read(pin_name p);
