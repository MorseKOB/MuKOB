/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FONTS_H
#define __FONTS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#define LOWBYTE(v)   ((unsigned char) (v))
#define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define FONT_WIDTH 9
#define FONT_HIEGHT 10
#define FONT_BIT_MASK 0x01FF
extern const uint16_t Font_Table[];

#ifdef __cplusplus
}
#endif
#endif // __FONTS_H
