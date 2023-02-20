/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FONT_10_16_H
#define _FONT_10_16_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#define FONT_WIDTH 10
#define FONT_HEIGHT 16

#define FONT_BIT_MASK 0x03FF

extern const uint16_t Font_Table[];

#ifdef __cplusplus
}
#endif
#endif // _FONT_10_16_H
