#pragma once

#include "PinDefs.hpp"

#include <vector>
#include <optional>

typedef enum SpiBus {
    SpiBus2,
    SpiBus3,
    SpiBus5,
} SpiBus;

constexpr int fPCLK = 108'000'000; // Hz

class SPI {
public:
    SPI(SpiBus spiBus, std::optional<PinName> cs = std::nullopt, int hz = 1'000'000);
    ~SPI();

    void format(int bits, int mode = 0);
    // TODO: make more clear is set in steps
    void frequency(int hz);

    void transmit(uint8_t data);
    void transmit(const std::vector<uint8_t>& data);
    void transmit(const uint8_t*, size_t size);

    uint8_t transmitReceive(uint8_t data);
    std::vector<uint8_t> transmitReceive(const std::vector<uint8_t>& data);
    void transmitReceive(const uint8_t* dataIn, uint8_t* dataOut, size_t size);

private:
    SPI_HandleTypeDef spiHandle = {};
    std::optional<PinName> chipSelect;

    uint32_t freqToPrescaler(int hz);
};