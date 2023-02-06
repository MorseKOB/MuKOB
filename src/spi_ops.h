/**
 * MuKOB SPI operations.
 * 
 * The SPI is common to 3 devices, so this provides routines 
 * to read/write to them in a coordinated way.
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
*/
#ifndef SPI_OPS_H
#define SPI_OPS_H
#ifdef __cplusplus
 extern "C" {
#endif
#include <stddef.h>
#include <stdbool.h>

void spi_begin(void);

void spi_end(void);

int spi_read(uint8_t txv, uint8_t *dst, size_t len);

int spi_write(const uint8_t *data, size_t len);

int spi_write16(const uint16_t *data, size_t len);

#ifdef __cplusplus
 }
#endif
#endif // SPI_OPS_H
