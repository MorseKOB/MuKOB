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
extern void spi_begin(spi_inst_t* spi) {
    // ZZZ - Employ a mutex for the SPI channel to make sure a single core has access.
}

/**
 * Called at the end of one or more SPI operations to signal that
 * the SPI is now available. `spi_begin` must have been called at
 * the beginning of the operations.
*/
extern void spi_end(spi_inst_t* spi) {
    // ZZZ - Employ a mutex for the SPI channel to make sure a single core has access.
}

extern int spi_read(spi_inst_t* spi, uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(spi, txv, dst, len));
}

extern int spi_write8_buf(spi_inst_t* spi, const uint8_t* buf, size_t len) {
    return (spi_write_blocking(spi, buf, len));
}

extern int spi_write8(spi_inst_t* spi, uint8_t data) {
    return (spi_write_blocking(spi, &data, 1));
}

extern int spi_write16(spi_inst_t* spi, uint16_t data) {
    size_t len = sizeof(uint16_t);
    uint8_t bytes[] = {(data & 0xff00) >> 8, data & 0xff};
    spi_write_blocking(spi, bytes, len);
    return (len);
}

extern int spi_write16_buf(spi_inst_t* spi, const uint16_t* buf, size_t len) {
    int written = 0;
    while (len--) {
        spi_write16(spi, *buf);
        buf++;
        written++;
    }
    return (written);
}

extern void spi_display_begin() {
    spi_begin(SPI_DISPLAY_DEVICE);
}

extern void spi_display_end() {
    spi_end(SPI_DISPLAY_DEVICE);
}

extern int spi_display_read(uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(SPI_DISPLAY_DEVICE, txv, dst, len));
}

extern int spi_display_write8_buf(const uint8_t* data, size_t len) {
    return (spi_write_blocking(SPI_DISPLAY_DEVICE, data, len));
}

extern int spi_display_write8(uint8_t data) {
    return (spi_write8(SPI_DISPLAY_DEVICE, data));
}

extern int spi_display_write16(uint16_t data) {
    return (spi_write16(SPI_DISPLAY_DEVICE, data));
}

extern int spi_display_write16_buf(const uint16_t* buf, size_t len) {
    return (spi_write16_buf(SPI_DISPLAY_DEVICE, buf, len));
}

extern void spi_touch_begin() {
    spi_begin(SPI_TOUCH_DEVICE);
    gpio_put(SPI_CS_TOUCH, SPI_CS_ENABLE);
}

extern void spi_touch_end() {
    gpio_put(SPI_CS_TOUCH, SPI_CS_ENABLE);
    spi_end(SPI_TOUCH_DEVICE);
}

extern int spi_touch_read(uint8_t txv, uint8_t* dst, size_t len) {
    return (spi_read_blocking(SPI_TOUCH_DEVICE, txv, dst, len));
}

extern int spi_touch_write_buf(const uint8_t* data, size_t len) {
    return (spi_write_blocking(SPI_TOUCH_DEVICE, data, len));
}

extern int spi_touch_write8(uint8_t data) {
    return (spi_write8(SPI_TOUCH_DEVICE, data));
}

extern int spi_touch_write16(uint16_t data) {
    return (spi_write16(SPI_TOUCH_DEVICE, data));
}

extern int spi_touch_write16_buf(const uint16_t* buf, size_t len) {
    return (spi_write16_buf(SPI_TOUCH_DEVICE, buf, len));
}
