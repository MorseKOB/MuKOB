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
#include "hardware/spi.h"

/**
 * Make sure we have control of the SPI for one or more operations.
 * `spi_end` must be called when the SPI is done being used.
*/
void spi_begin(void) {

}

/**
 * Called at the end of one or more SPI operations to signal that 
 * the SPI is now available. `spi_begin` must have been called at 
 * the beginning of the operations.
*/
void spi_end(void) {

}

int spi_read(uint8_t txv, uint8_t *dst, size_t len) {
    int read = spi_read_blocking(SPI_DEVICE, txv, dst, len);

    return (read);
}

int spi_write(const uint8_t *src, size_t len) {
    int written = spi_write_blocking(SPI_DEVICE, src, len);

    return (written);
}