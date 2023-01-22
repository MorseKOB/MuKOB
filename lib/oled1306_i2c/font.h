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

//ASCII
typedef struct _tFont
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  
} sFONT;


//extern sFONT Font;
extern const uint16_t Font_Table[];

#ifdef __cplusplus
}
#endif
#endif /* __FONTS_H */
