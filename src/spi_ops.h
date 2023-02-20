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
#ifndef SPI_OPS_H
#define SPI_OPS_H
#ifdef __cplusplus
 "C" {
#endif
#include <stddef.h>
#include <stdbool.h>
#include "hardware/spi.h"

void spi_begin(spi_inst_t* spi);
void spi_display_begin(void);
void spi_tsd_begin(void);

void spi_end(spi_inst_t* spi);
void spi_display_end(void);
void spi_tsd_end(void);

int spi_read(spi_inst_t* spi, uint8_t txv, uint8_t* dst, size_t len);
int spi_display_read(uint8_t txv, uint8_t* dst, size_t len);
int spi_tsd_read(uint8_t txv, uint8_t* dst, size_t len);

int spi_write(spi_inst_t* spi, const uint8_t* data, size_t len);
int spi_display_write(const uint8_t* data, size_t len);
int spi_tsd_write(const uint8_t* data, size_t len);

int spi_write16(spi_inst_t* spi, const uint16_t* data, size_t len);
int spi_display_write16(const uint16_t* data, size_t len);
int spi_tsd_write16(const uint16_t* data, size_t len);

#ifdef __cplusplus
 }
#endif
#endif // SPI_OPS_H
