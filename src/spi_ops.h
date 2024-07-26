/**
 * MuKOB SPI operations.
 *
 * The SPI is used by 3 devices:
 *  Display (on one of the HW SPIs)
 *  Touch (on HW SPI with SD Card)
 *  SD Card (on HW SPI with Touch)
 *
 * The Display and Touch are on different HW SPI so the touch can be read indepentent
 * of the display being updated.
 *
 * These functions allow the SPIs
 * to be read/writen in a coordinated way.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _SPI_OPS_H_
#define _SPI_OPS_H_
#ifdef __cplusplus
 "C" {
#endif
#include <stddef.h>
#include <stdbool.h>
#include "hardware/spi.h"

#define SPI_HIGH_TXD_FOR_READ 0xFF
#define SPI_LOW_TXD_FOR_READ 0x00

extern void spi_begin(spi_inst_t* spi);
extern void spi_display_begin(void);
extern void spi_touch_begin(void);

extern void spi_end(spi_inst_t* spi);
extern void spi_display_end(void);
extern void spi_touch_end(void);

extern int spi_read(spi_inst_t* spi, uint8_t txval, uint8_t* dst, size_t len);
extern int spi_display_read(uint8_t txval, uint8_t* dst, size_t len);
extern int spi_touch_read(uint8_t txval, uint8_t* dst, size_t len);

extern int spi_write8_buf(spi_inst_t* spi, const uint8_t* buf, size_t len);
extern int spi_display_write8_buf(const uint8_t* buf, size_t len);
extern int spi_touch_write_buf(const uint8_t* buf, size_t len);

extern int spi_write8(spi_inst_t* spi, uint8_t data);
extern int spi_display_write8(uint8_t data);
extern int spi_touch_write8(uint8_t data);

extern int spi_write16(spi_inst_t* spi, uint16_t data);
extern int spi_display_write16(uint16_t data);
extern int spi_touch_write16(uint16_t data);

extern int spi_write16_buf(spi_inst_t* spi, const uint16_t* buf, size_t len);
extern int spi_display_write16_buf(const uint16_t* buf, size_t len);
extern int spi_touch_write16_buf(const uint16_t* buf, size_t len);

#ifdef __cplusplus
 }
#endif
#endif // _SPI_OPS_H_
