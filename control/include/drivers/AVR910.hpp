/**
 * @author Aaron Berk
 *
 * @section LICENSE
 *
 * Copyright (c) 2010 Aaron Berk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Program AVR chips with the AVR910 ISP (in-system programming) protocol,
 * using an mbed.
 *
 * AVR910 Application Note:
 *
 * http://www.atmel.com/dyn/resources/prod_documents/doc0943.pdf
 */
/**
 * Modified by the Georgia Tech RoboJackets
 */

#pragma once

#include "robocup.hpp"
#include <memory>
#include <cstdio>

// AVR SPI Commands
#define ATMEL_VENDOR_CODE 0x1E
#define DEVICE_LOCKED 0x00
#define WRITE_HIGH_BYTE 0x48
#define WRITE_LOW_BYTE 0x40
#define READ_HIGH_BYTE 0x28
#define READ_LOW_BYTE 0x20
#define WRITE_HIGH_FLASH_BYTE 0x68
#define WRITE_LOW_FLASH_BYTE 0x60

#define AVR_FAMILY_MASK 0xF0
#define AVR_FAMILY_ID 0x90
#define ATMEGA_DEVICE_ID 0x02
#define ATMEGA_PAGESIZE 64  // Size in words (word = 2 bytes)
#define ATMEGA_NUM_PAGES 256

/**
 * @brief AVR910 ISP
 *
 * This class facilitates loading a program onto an AVR chip's flash memory.
 */
class AVR910 {
public:
    /**
     * Constructor.
     *
     * @param spi SPI bus being used for this device
     * @param nCs Chip select pin on mtrain
     * @param nReset mtrain pin for not reset line on the ISP interface.
     *
     */
    AVR910(LockedStruct<SPI>& spi, std::shared_ptr<DigitalOut> nCs, PinName nReset);

    /**
     * Sends an enable programming command, allowing device registers to be
     * read and commands sent.
     *
     * @return true if initialization completed successfully
     */
    bool init();

    /**
     * Program the AVR microcontroller connected to the mbed.
     *
     * Sends a chip erase command followed by writing the binary to the AVR
     * page buffer and writing the page buffer to flash memory whenever it is
     * full.
     *
     * @param binary File pointer to the binary file to be loaded onto the
     *               AVR microcontroller.
     * @param pageSize The size of one page on the device in words. If the
     *                 device does not use paged memory, set this as the size
     *                 of memory of the device in words.
     * @param numPages The number of pages on the device. If the device does
     *                 not use paged memory, set this to 1 (default).
     *
     * @return  boolean value indicating success
     */
    bool program(FILE* binary, int pageSize, int numPages = 1);

    /**
     * Program the AVR microcontroller connected to the mbed.
     *
     * Sends a chip erase command followed by writing the binary to the AVR
     * page buffer and writing the page buffer to flash memory whenever it is
     * full.
     *
     * @param binary Array pointer to the start binary to be loaded onto the
     *               AVR microcontroller
     * @param length Length of the binary array being loaded onto the AVR
     *               microcontroller
     * @param pageSize The size of one page on the device in words. If the
     *                 device does not use paged memory, set this as the size
     *                 of memory of the device in words.
     * @param numPages The number of pages on the device. If the device does
     *                 not use paged memory, set this to 1 (default).
     *
     * @return  boolean value indicating success
     */
    bool program(const uint8_t* binary, unsigned int length, int pageSize, int numPages = 1);

    /**
     * Read the vendor code of the device.
     *
     * @return The vendor code - should be 0x1E for Atmel.
     *         0x00 -> Device is locked.
     */
    int readVendorCode();

    /**
     * Read the part family and flash size of the device.
     *
     * @return Code indicating the family of AVR microcontrollers the device
     *     comes from and how much flash memory it contains. 0xFF -> Device code
     *     erased or target missing. 0x01 -> Device is locked.
     */
    int readPartFamilyAndFlashSize();

    /**
     * Read the part number.
     *
     * @return Code identifying the part number.
     *         0xFF -> Device code erased or target missing.
     *         0x02 -> Device is locked.
     */
    int readPartNumber();

protected:
    void writeFuseBitsLow();

    int readRegister(int reg);

    /**
     * Check the binary has been written correctly.
     *
     * @param numPages The number of pages written to the AVR microcontroller.
     * @param pageSize The size of a page in words
     * @param binary File pointer to the binary used.
     *
     * @return boolean indicating success
     */
    bool checkMemory(int numPages, int pageSize, FILE* binary,
                     bool verbose = false);

    /**
     * Check the binary has been written correctly.
     *
     * @param numPages The number of pages written to the AVR microcontroller.
     * @param pageSize The size of a page in words
     * @param binary Byte array pointer to the binary used
     * @param length Length of the binary byte array used
     *
     * @return boolean indicating success
     */
    bool checkMemory(int numPages, int pageSize, const uint8_t* binary,
                     unsigned int length, bool verbose = false);

    /**
     * Brings the reset line high to exit programming mode.  The program()
     * method does this automatically, but this can be called instead of
     * program() to return the AVR to its normal state.
     */
    void exitProgramming();

    LockedStruct<SPI>::Lock lock_spi() {
        // Lock the SPI struct. If it's the first time it's been locked, we also
        // need to set the frequency.
        bool set_freq;
        auto spi_lock = spi_.lock(&set_freq);
        if (set_freq) {
            spi_lock->frequency(100'000);
        }
        return spi_lock;
    }

    LockedStruct<SPI>& spi_;
    std::shared_ptr<DigitalOut> nCs_;
    DigitalOut nReset_;

private:
    /**
     * Issue an enable programming command to the AVR microcontroller.
     *
     * @param boolean indicating success
     */
    bool enableProgramming();

    /**
     * Poll the device until it has finished its current operation.
     */
    void poll(int high_low, char page_number, char page_offset);

    /**
     * Issue a chip erase command to the AVR microcontroller.
     */
    void chipErase();

    /**
     * Load a byte into the memory page buffer.
     *
     * @param highLow Indicate whether the byte being loaded is a high or low
     *                byte.
     * @param data The data byte to load.
     */
    void loadMemoryPage(int highLow, char address, char data);

    /**
     * Write a byte into the flash memory.
     *
     * @param highLow Indicate whether the byte being loaded is a high or low
     *                byte.
     * @param address The address to load the byte at.
     * @param data The data byte to load.
     */
    void writeFlashMemoryByte(int highLow, int address, char data);

    /**
     * Write the memory page buffer to flash memory.
     *
     * @param pageNumber The page number to write to in flash memory.
     */
    void writeFlashMemoryPage(char pageNumber, char lc_offset, int lc_highLow);

    /**
     * Read a byte from program memory.
     *
     * @param highLow Indicate whether the byte being read is a low or high
     * byte.
     * @param pageNumber The page number to read from.
     * @param pageOffset Address of byte in the page.
     *
     * @return The byte at the specified memory location.
     */
    char readProgramMemory(int highLow, char pageNumber, char pageOffset);
};
