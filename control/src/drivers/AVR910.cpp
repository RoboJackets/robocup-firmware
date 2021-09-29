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

#include "drivers/AVR910.hpp"
#include "delay.hpp"

using namespace std;

AVR910::AVR910(LockedStruct<SPI>& spi, std::shared_ptr<DigitalOut> nCs, PinName nReset)
    : spi_(spi), nCs_(nCs), nReset_(nReset) {
}

bool AVR910::init() {
    int tryCnt = 0;
    bool enabled = false;
    nCs_->write(1);

    do {
        // Give nReset a positive pulse for at least two CPU clock cycles
        nReset_ = 1;
        vTaskDelay(100);
        nReset_ = 0;

        // Wait at least 20 ms
        vTaskDelay(100);

        // Enable SPI Serial Programming
        // may not be synced so toggle and try again
        enabled = enableProgramming();
    } while (!enabled && tryCnt < 20);

    if (!enabled) {
        printf(
                "ERROR: AVR910 unable to enable programming mode for chip.  "
                "Further commands will fail\r\n");
    }

    return enabled;
}

bool AVR910::program(FILE* binary, int pageSize, int numPages) {
    // Clear memory contents.
    chipErase();

    char pageOffset = 0;
    int pageNumber = 0;
    int address = 0;
    int c = 0;
    int highLow = 0;

    char lc_offset = 0xFF;
    int lc_highlow = 0xFF;

    fseek(binary, 0, SEEK_SET);
    // We're dealing with paged memory.
    if (numPages > 1) {
        while ((c = getc(binary)) != EOF) {
            // Page is fully loaded, time to write it to flash.
            if (pageOffset == (pageSize)) {
                writeFlashMemoryPage(pageNumber, lc_offset, lc_highlow);
                lc_offset = 0xFF;
                lc_highlow = 0xFF;

                pageNumber++;
                if (pageNumber > numPages) {
                    printf(
                        "ERROR: AVR910 binary exceeds chip memory "
                        "capacity\r\n");
                    return false;
                }
                pageOffset = 0;
            }

            if (lc_offset == 0xFF && c != 0xFF) {
                lc_offset = pageOffset;
                if (!highLow) {
                    lc_highlow = READ_LOW_BYTE;
                } else {
                    lc_highlow = READ_HIGH_BYTE;
                }
            }

            // Write low byte.
            if (highLow == 0) {
                loadMemoryPage(WRITE_LOW_BYTE, pageOffset, c);
                highLow = 1;
                // printf("Writing low\r\n");
            }
            // Write high byte.
            else {
                loadMemoryPage(WRITE_HIGH_BYTE, pageOffset, c);
                highLow = 0;
                pageOffset++;
            }
            // printf("PageNumber: %d\r\n", pageNumber);
        }

    } else {
        // We're dealing with non-paged memory.

        while ((c = getc(binary)) != EOF) {
            // Write low byte.
            if (highLow == 0) {
                writeFlashMemoryByte(WRITE_LOW_FLASH_BYTE, address, c);
                highLow = 1;
            } else {
                // Write high byte.
                writeFlashMemoryByte(WRITE_HIGH_FLASH_BYTE, address, c);
                highLow = 0;
                address++;

                // Page size is our memory size in the non-paged memory case.
                // Therefore if we've gone beyond our size break because we
                // don't have any more room.
                if (address > pageSize) {
                    printf(
                        "ERROR: AVR910 binary exceeds chip memory "
                        "capacity\r\n");
                    return false;
                }
            }
        }
    }

    // We might have partially filled up a page.
    writeFlashMemoryPage(pageNumber, lc_offset, lc_highlow);

    bool success = checkMemory(pageSize, pageNumber + 1, binary, true);

    // Leave serial programming mode by toggling reset
    exitProgramming();

    return success;
}

bool AVR910::program(const uint8_t* binary, unsigned int length, int pageSize, int numPages) {
    // Clear memory contents.
    // should automatically be cleared
    chipErase();

    char pageOffset = 0;
    int pageNumber = 0;
    int address = 0;
    int c = 0;
    int highLow = 0;
    unsigned int binaryLoc = 0;

    char lc_offset = 0xFF;
    int lc_highlow = 0xFF;

    // We're dealing with paged memory.
    if (numPages > 1) {
        while (binaryLoc < length) {
            c = binary[binaryLoc];
            binaryLoc++;

            // Page is fully loaded, time to write it to flash.
            // printf("page size: %d\r\n", pageSize);
            // printf("page offset: %d\r\n", pageOffset);
            if (pageOffset == (pageSize)) {
                writeFlashMemoryPage(pageNumber, lc_offset, lc_highlow);
                lc_offset = 0xFF;
                lc_highlow = 0xFF;

                pageNumber++;
                if (pageNumber > numPages) {
                    printf(
                        "ERROR: AVR910 binary exceeds chip memory "
                        "capacity\r\n");
                    return false;
                }
                pageOffset = 0;
            }

            if (lc_offset == 0xFF && c != 0xFF) {
                lc_offset = pageOffset;
                if (highLow == 0) {
                    lc_highlow = READ_LOW_BYTE;
                } else {
                    lc_highlow = READ_HIGH_BYTE;
                }
            }

            // Write low byte.
            if (highLow == 0) {
                loadMemoryPage(WRITE_LOW_BYTE, pageOffset, c);
                highLow = 1;
                // printf("Writing low\r\n");
            }
            // Write high byte.
            else {
                loadMemoryPage(WRITE_HIGH_BYTE, pageOffset, c);
                highLow = 0;
                pageOffset++;
                // printf("Writing high\r\n");
            }
            // printf("PageNumber: %d\r\n", pageNumber);
        }
    } else {
        // We're dealing with non-paged memory.

        while (binaryLoc < length) {
            c = binary[binaryLoc];
            binaryLoc++;

            // Write low byte.
            if (highLow == 0) {
                writeFlashMemoryByte(WRITE_LOW_FLASH_BYTE, address, c);
                highLow = 1;
            } else {
                // Write high byte.
                writeFlashMemoryByte(WRITE_HIGH_FLASH_BYTE, address, c);
                highLow = 0;
                address++;

                // Page size is our memory size in the non-paged memory case.
                // Therefore if we've gone beyond our size break because we
                // don't have any more room.
                if (address > pageSize) {
                    printf(
                        "ERROR: AVR910 binary exceeds chip memory "
                        "capacity\r\n");
                    return false;
                }
            }
        }
    }

    // We might have partially filled up a page.
    writeFlashMemoryPage(pageNumber, lc_offset, lc_highlow);

    bool success = checkMemory(pageSize, pageNumber + 1, binary, length, true);

    // Leave serial programming mode by toggling reset
    exitProgramming();

    return success;
}

bool AVR910::enableProgramming() {
    auto spi_lock = lock_spi();

    // Programming Enable Command: 0xAC, 0x53, 0x00, 0x00
    // Byte two echo'd back in byte three.
    DWT_Delay(10);
    nCs_->write(0);
    spi_lock->transmit(0xAC);
    spi_lock->transmit(0x53);
    int response = spi_lock->transmitReceive(0x00);
    spi_lock->transmit(0x00);
    nCs_->write(1);
    DWT_Delay(10);

    if (response == 0x53) {
        return true;
    } else {
        return false;
    }
}

void AVR910::poll(int high_low, char page_number, char page_offset) {
    // auto spi_lock = lock_spi();
    //
    // // Query the chip until it indicates it's ready by setting the busy bit to 0
    // int response = 0;
    // nCs_->write(0);
    // do {
    //     spi_lock->transmit(0xF0);
    //     spi_lock->transmit(0x00);
    //     spi_lock->transmit(0x00);
    //     response = spi_lock->transmitReceive(0x00);
    // } while ((response & 0x01) != 0);
    // nCs_->write(0);
    vTaskDelay(5);
}

int AVR910::readRegister(int reg) {
    DWT_Delay(10);
    auto spi_lock = lock_spi();

    nCs_->write(0);
    spi_lock->transmit(0x30);
    spi_lock->transmit(0x00);
    spi_lock->transmit(reg);
    int val = spi_lock->transmitReceive(0x00);
    nCs_->write(1);
    DWT_Delay(10);

    return val;
}

int AVR910::readVendorCode() {
    // Issue read signature byte command.
    // Address 0x00 is vendor code.
    return readRegister(0x00);
}

int AVR910::readPartFamilyAndFlashSize() {
    // Issue read signature byte command.
    // Address 0x01 is part family and flash size code.
    return readRegister(0x01);
}

int AVR910::readPartNumber() {
    // Issue read signature byte command.
    // Address 0x02 is part number code.
    return readRegister(0x02);
}

void AVR910::chipErase() {
    auto spi_lock = lock_spi();
    DWT_Delay(10);
    // Issue chip erase command.
    nCs_->write(0);
    spi_lock->transmit(0xAC);
    spi_lock->transmit(0x80);
    spi_lock->transmit(0x00);
    spi_lock->transmit(0x00);
    nCs_->write(1);
    DWT_Delay(10);

    vTaskDelay(15); // 9 ms min
}

/**
 * Load program memory page
 *
 * @param highLog first byte of the command, whether low or high byte
 *                0100 0000 for low byte
 *                0100 1000 for high byte
 * @param address 6 lsbs of the word address
 *                AKA location in the current page
 * @param data byte of data to be stored
 *
 * @note load low before high
 */
void AVR910::loadMemoryPage(int highLow, char address, char data) {
    auto spi_lock = lock_spi();

    // Load program memory page command
    // Write H (high or low) data i to Program
    // Memory page at word address b. Data
    // low byte must be loaded before Data
    // high byte is applied within the same
    // address.
    DWT_Delay(10);
    nCs_->write(0);
    spi_lock->transmit(highLow); // 0100 H000
    spi_lock->transmit(0x00); // 00xx xxxx
    spi_lock->transmit(address & 0x3F); // xxxb bbbb
    spi_lock->transmit(data); // iiii iiii
    nCs_->write(1);
    DWT_Delay(10);
}

void AVR910::writeFlashMemoryByte(int highLow, int address, char data) {
    auto spi_lock = lock_spi();

    DWT_Delay(10);
    nCs_->write(0);
    spi_lock->transmit(0x4C);
    spi_lock->transmit(address & 0xFF00 >> 8);
    spi_lock->transmit(address & 0x003F);
    spi_lock->transmit(data);
    nCs_->write(1);
    DWT_Delay(10);
}

void AVR910::writeFuseBitsLow() {
    auto spi_lock = lock_spi();

    DWT_Delay(10);
    nCs_->write(0);
    spi_lock->transmit(0xAC);
    spi_lock->transmit(0xA0);
    spi_lock->transmit(0x33);
    spi_lock->transmit(0xE4);
    nCs_->write(1);
    DWT_Delay(10);
}

/**
 * Write program memory page
 *
 * @param pageNumber page number to write
 */
void AVR910::writeFlashMemoryPage(char pageNumber, char pageOffset, int highlow) {
    auto spi_lock = lock_spi();

    // Write program memory page command
    // Write Program Memory Page at
    // address a:b.
    DWT_Delay(10);
    nCs_->write(0);
    spi_lock->transmit(0x4C); // 0100 1100
    // 11 bits total, 5 for page offset, 6 for page number
    spi_lock->transmit(pageNumber >> 2); // 0000 0000
    spi_lock->transmit(pageNumber << 6); // 0x00aa aaaa
    spi_lock->transmit(0x00); // xxxx xxxx
    nCs_->write(1);
    DWT_Delay(10);

    poll(highlow, pageNumber, pageOffset);
}

/**
 * Read program memory
 *
 * @param highLow First byte of command
 *                0100 0000 for low byte
 *                0100 0100 for high byte
 * @param pageNumber Page number from [0 - N]
 * @param pageOffset location in words in page to read
 *
 * @return value read
 */
char AVR910::readProgramMemory(int highLow, char pageNumber, char pageOffset) {
    auto spi_lock = lock_spi();

    // Read program memory command
    // Read H (high or low) data o from
    // Program memory at word address a:b
    nCs_->write(0);
    spi_lock->transmit(highLow); // 0100 0H00
    spi_lock->transmit(pageNumber >> 2); // 00aa aaaa
    spi_lock->transmit((pageNumber << 6) | (pageOffset & 0x3F)); // aabb bbbb
    char response = spi_lock->transmitReceive(0x00); // oooo oooo
    nCs_->write(1);

    return response;
}

bool AVR910::checkMemory(int pageSize, int numPages, FILE* binary,
                         bool verbose) {
    bool success = true;

    printf("Checking memory? (pagesize: %d, numpages: %d) \r\n", pageSize,
           numPages);

    // Go back to the beginning of the binary file.
    fseek(binary, 0, SEEK_SET);

    for (int page = 0; page < numPages; page++) {
        for (int offset = 0; offset < pageSize; offset++) {
            int response;
            char c = getc(binary);
            // Read program memory low byte.
            response = readProgramMemory(READ_LOW_BYTE, page, offset);

            if (c != response) {
                if (verbose || true) {
                    printf("Page %i low byte %i: 0x%02x : ", page, offset,
                           response);
                    printf("Correct byte is 0x%02x\r\n", c);
                } else {
                    return false;
                }
                success = false;
            }
            c = getc(binary);
            // Read program memory high byte.
            response = readProgramMemory(READ_HIGH_BYTE, page, offset);

            if (c != response) {
                if (verbose || true) {
                    printf("Page %i high byte %i: 0x%02x : ", page, offset,
                           response);
                    printf("Correct byte is 0x%02x\r\n", c);
                } else {
                    return false;
                }
                success = false;
            }
        }
    }

    if (verbose) {
        if (success) {
            printf("Kicker Memory Contents: OK.\r\n");
        } else {
            printf("Kicker Memory Contents: FAILED.\r\n");
        }
    }

    return success;
}

bool AVR910::checkMemory(int pageSize, int numPages, const uint8_t* binary,
                         unsigned int length, bool verbose) {
    bool success = true;

    printf("Checking memory? pagesize: %d, numpages: %d \r\n", pageSize,
           numPages);

    unsigned int binaryLoc = 0;

    for (int page = 0; page < numPages; page++) {
        for (int offset = 0; offset < pageSize; offset++) {
            int response;
            char c = binary[binaryLoc];
            binaryLoc++;

            if (binaryLoc >= length) {
                printf("Done reading\r\n");
                break;
            }

            // Read program memory low byte.
            response = readProgramMemory(READ_LOW_BYTE, page, offset);

            if (c != response) {
                if (verbose) {
                    printf("Page %i low byte %i: 0x%02x\r\n", page, offset,
                           response);
                    printf("Correct byte is 0x%02x\r\n", c);
                } else {
                    return false;
                }
                success = false;
            }

            c = binary[binaryLoc];
            binaryLoc++;

            if (binaryLoc >= length)
                break;

            // Read program memory high byte.
            response = readProgramMemory(READ_HIGH_BYTE, page, offset);

            if (c != response) {
                if (verbose) {
                    printf("Page %i high byte %i: 0x%02x\r\n", page, offset,
                           response);
                    printf("Correct byte is 0x%02x\r\n", c);
                } else {
                    return false;
                }
                success = false;
            }
        }
    }

    return success;
}

void AVR910::exitProgramming() {
    nReset_.write(0);
    // TODO(Kyle): Why wasn't vTaskDelay working here?
    DWT_Delay(100000);
    nReset_.write(1);
}
