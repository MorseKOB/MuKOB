/**
 * Copyright 2023 AESilky
 *
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef DISPLAY_ILI9341_H
#define DISPLAY_ILI9341_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include "ili9341_spi.h"
#include "font_10_16.h"

#define DISP_CHAR_COLS (ILI9341_WIDTH / FONT_WIDTH)
#define DISP_CHAR_LINES (ILI9341_HEIGHT / FONT_HEIGHT)

/** @brief Text character data for the full text screen */
extern uint8_t full_screen_text[DISP_CHAR_LINES * DISP_CHAR_COLS];
/** @brief Character color data for the full text screen */
extern uint8_t full_screen_color[DISP_CHAR_LINES * DISP_CHAR_COLS];


#ifdef __cplusplus
}
#endif
#endif // DISPLAY_ILI9341_H

