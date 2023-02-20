/**
 * MuKOB SPI operations.
 *
 * The SPI is used by 3 devices, so this provides routines
 * to read/write to them in a coordinated way.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "system_defs.h"
#include "spi_ops.h"

/**
 * Make sure we have control of the SPI for one or more operations.
 * `spi_end` must be called when the SPI is done being used.
*/
void spi_begin(spi_inst_t* spi) {

}

/**
 * Called at the end of one or more SPI operations to signal that
 * the SPI is now available. `spi_begin` must have been called at
 * the beginning of the operations.
*/
void spi_end(spi_inst_t* spi) {

}

int spi_read(spi_inst_t* spi, uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(spi, txv, dst, len));
}

int spi_write(spi_inst_t* spi, const uint8_t* data, size_t len) {
    return (spi_write_blocking(spi, data, len));
}

int spi_write16(spi_inst_t* spi, const uint16_t* data, size_t len) {
    for (int i = 0; i < len; i++) {
        uint8_t bytes[] = {(*data & 0xff00) >> 8, *data & 0xff};
        spi_write_blocking(spi, bytes, 2);
        data++;
    }
    return (len);
}

void spi_display_begin() {
    spi_begin(SPI_DISPLAY_DEVICE);
}

void spi_display_end() {
    spi_end(SPI_DISPLAY_DEVICE);
}

int spi_display_read(uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(SPI_DISPLAY_DEVICE, txv, dst, len));
}

int spi_display_write(const uint8_t* data, size_t len) {
    return (spi_write_blocking(SPI_DISPLAY_DEVICE, data, len));
}

int spi_display_write16(const uint16_t* data, size_t len) {
    return (spi_write16(SPI_DISPLAY_DEVICE, data, len));
}

void spi_tsd_begin() {
    spi_begin(SPI_TSD_DEVICE);
}

void spi_tsd_end() {
    spi_end(SPI_TSD_DEVICE);
}

int spi_tsd_read(uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(SPI_DISPLAY_DEVICE, txv, dst, len));
}

int spi_tsd_write(const uint8_t* data, size_t len) {
    return (spi_write_blocking(SPI_DISPLAY_DEVICE, data, len));
}

int spi_tsd_write16(const uint16_t* data, size_t len) {
    return (spi_write16(SPI_DISPLAY_DEVICE, data, len));
}
