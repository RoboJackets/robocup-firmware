#include "mtrain.hpp"

class DigitalIn {
public:
    DigitalIn(PinName pin, PullType pull = PullType::PullNone);

    bool read();

    operator bool() {
        return read();
    }

protected:
    PinName pin;
};