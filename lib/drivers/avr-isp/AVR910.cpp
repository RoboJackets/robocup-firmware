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

#include "AVR910.hpp"

using namespace std;

AVR910::AVR910(shared_ptr<SharedSPI> spi, PinName nCs, PinName nReset)
    : SharedSPIDevice(spi, nCs, true), nReset_(nReset) {
    setSPIFrequency(100000);

    // Enter serial programming mode by pulling reset line low.
    nReset_ = 0;

    // Wait 20ms before issuing first command.
    Thread::wait(20);

    // Enable programming mode on the chip
    // It's possible for it to fail, so try it a few times.
    bool enabled = false;
    for (int i = 0; i < 10; i++) {
        enabled = enableProgramming();
        if (enabled) break;

        // Give nReset a positive pulse.
        nReset_ = 1;
        Thread::wait(20);
        nReset_ = 0;
        Thread::wait(20);
    }

    if (!enabled) {
        printf(
            "ERROR: AVR910 unable to enable programming mode for chip.  "
            "Further commands will fail\r\n");
    }
}

bool AVR910::program(FILE* binary, int pageSize, int numPages) {
    // Clear memory contents.
    chipErase();

    char pageOffset = 0;
    int pageNumber = 0;
    int address = 0;
    int c = 0;
    int highLow = 0;

    fseek(binary, 0, SEEK_SET);
    // We're dealing with paged memory.
    if (numPages > 1) {
        while ((c = getc(binary)) != EOF) {
            // Page is fully loaded, time to write it to flash.
            // printf("page size: %d\r\n", pageSize);
            // printf("page offset: %d\r\n", pageOffset);
            if (pageOffset == (pageSize)) {
                writeFlashMemoryPage(pageNumber);

                pageNumber++;
                if (pageNumber > numPages) {
                    printf(
                        "ERROR: AVR910 binary exceeds chip memory "
                        "capacity\r\n");
                    return false;
                }
                pageOffset = 0;
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
    writeFlashMemoryPage(pageNumber);

    bool success = checkMemory(pageSize, pageNumber + 1, binary, true);

    // Leave serial programming mode by toggling reset
    exitProgramming();

    return success;
}

bool AVR910::enableProgramming() {
    // Programming Enable Command: 0xAC, 0x53, 0x00, 0x00
    // Byte two echo'd back in byte three.
    chipSelect();
    m_spi->write(0xAC);
    m_spi->write(0x53);
    int response = m_spi->write(0x00);
    m_spi->write(0x00);
    chipDeselect();

    if (response == 0x53) {
        return true;
    } else {
        return false;
    }
}

void AVR910::poll() {
    // Query the chip until it indicates it's ready by setting the busy bit to 0
    int response = 0;
    chipSelect();
    do {
        m_spi->write(0xF0);
        m_spi->write(0x00);
        m_spi->write(0x00);
        response = m_spi->write(0x00);
    } while ((response & 0x01) != 0);
    chipDeselect();
}

int AVR910::readRegister(int reg) {
    chipSelect();
    m_spi->write(0x30);
    m_spi->write(0x00);
    m_spi->write(reg);
    int val = m_spi->write(0x00);
    chipDeselect();

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
    // Issue chip erase command.
    chipSelect();
    m_spi->write(0xAC);
    m_spi->write(0x80);
    m_spi->write(0x00);
    m_spi->write(0x00);
    chipDeselect();

    poll();

    // Temporarily release reset line.
    nReset_ = 1;
    nReset_ = 0;
}

void AVR910::loadMemoryPage(int highLow, char address, char data) {
    chipSelect();
    m_spi->write(highLow);
    m_spi->write(0x00);
    // m_spi->write(address & 0x3F);
    m_spi->write(address &
                 0x3F);  // flash has 64 words, so 6 bits to address all
    m_spi->write(data);
    chipDeselect();

    poll();
}

void AVR910::writeFlashMemoryByte(int highLow, int address, char data) {
    chipSelect();
    m_spi->write(highLow);
    m_spi->write(address & 0xFF00 >> 8);
    m_spi->write(address & 0x00FF);
    m_spi->write(data);
    chipDeselect();
}

//
// 12,11,10,9,8,7,6,5,4,3,2,1,0
void AVR910::writeFlashMemoryPage(char pageNumber) {
    chipSelect();
    m_spi->write(0x4C);
    // m_spi->write(0x00);
    // 13 bits total, 6 for page offset, 7 for page number
    // top 5 bits stored in bottom of byte
    m_spi->write(pageNumber >> 2);
    // top 2 bits stored in top of byte
    m_spi->write(pageNumber << 6);
    // m_spi->write(pageNumber >> 3);  // top 5 bits stored in bottom of byte
    // m_spi->write(pageNumber << 5);  // bottom 3 bits stored in top of byte
    m_spi->write(0x00);
    chipDeselect();

    poll();
}

char AVR910::readProgramMemory(int highLow, char pageNumber, char pageOffset) {
    chipSelect();
    m_spi->write(highLow);
    m_spi->write(pageNumber >> 2);
    m_spi->write((pageNumber << 6) | (pageOffset & 0x3F));
    char response = m_spi->write(0x00);
    chipDeselect();

    poll();

    return response;
}

bool AVR910::checkMemory(int pageSize, int numPages, FILE* binary,
                         bool verbose) {
    bool success = true;

    printf("Checking memory? pagesize: %d, numpages: %d \r\n", pageSize,
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
                if (verbose) {
                    printf("Page %i low byte %i: 0x%02x\r\n", page, offset,
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
    nReset_ = 0;
    Thread::wait(20);
    nReset_ = 1;
}
