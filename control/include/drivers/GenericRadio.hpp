#pragma once

#include <cstdint>

class GenericRadio {
public:
    /**
     * Blocking call to send X number of bytes from `data` over the radio
     *
     * @param data raw array of data to send over the radio
     * @param numBytes number of bytes in the array to send
     *
     * @return number of bytes sent
     *
     * @note It is assumed that `data` is at least X bytes long
     */
    virtual unsigned int send(const uint8_t* data, const unsigned int numBytes) = 0;

    /**
     * Blocking call to write up to `maxNumBytes` into `data` from the radio
     *
     * @param data raw array that will be filled with data that was sent over the radio
     * @param maxNumBytes max number of bytes to write into `data` from the radio
     *
     * @return actual number of bytes written to data
     *
     * @note The radio may or may not keep the rest of the message in the buffer
     *  if the entire message is not read. It is undefined behavior based on
     *  the specific device
     */
    virtual unsigned int receive(uint8_t* data, const unsigned int maxNumBytes) = 0;

    /**
     * Returns true when there is data to read from the radio
     */
    virtual bool isAvailable() = 0;

    /**
     * Returns whether the radio is connected to WiFi or not
     */
    virtual bool isConnected() = 0;

    /**
     * Returns whether the radio is properly initialized or not
     */
    virtual bool isInitialized() = 0;

    /**
     * Resets the device and connects to the network
     */
    virtual void reset() = 0;
};
