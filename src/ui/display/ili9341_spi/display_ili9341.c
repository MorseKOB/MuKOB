/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "system_defs.h"
#include "mukboard.h"

#include "display_ili9341.h"
#include "font_10_16.h"
#include <stdio.h>
#include <string.h>

#define CHAR_HIEGHT 16
#define CHAR_WIDTH  10

/*! \brief Map of Color24 (RGB) values indexed by Color16 numbers. */
uint32_t _color16_map[] = {
    ILI9341_BLACK,
    ILI9341_BLUE,
    ILI9341_GREEN,
    ILI9341_CYAN,
    ILI9341_RED,
    ILI9341_MAGENTA,
    ILI9341_BROWN,
    ILI9341_WHITE,
    ILI9341_GREY,
    ILI9341_LT_BLUE,
    ILI9341_LT_GREEN,
    ILI9341_LT_CYAN,
    ILI9341_ORANGE,
    ILI9341_LT_MAGENTA,
    ILI9341_YELLOW,
    ILI9341_BR_WHITE
};

#define C24_MASK_RED    0x00FF0000
#define C24_SHIFT_RED   16
#define C24_MASK_GREEN  0x0000FF00
#define C24_SHIFT_GREEN 8
#define C24_MASK_BLUE   0x000000FF
#define C24_SHIFT_BLUE  0

/* Dirty text rows */
static bool __dirty_text_rows[DISP_CHAR_ROWS]; 
/* Default forground color */
static uint8_t __fg;
/* Default background color */
static uint8_t __bg;

/* Cursor position */
static scr_position_t __cursor_pos;

/* Margin boundaries */
static unsigned int __margin_bottom;
static unsigned int __margin_left;
static unsigned int __margin_right;
static unsigned int __margin_top;

/* Memory display_full_area for a screen of text (characters) */
uint8_t full_screen_text[DISP_CHAR_COLS * DISP_CHAR_ROWS];
/* Memory display_full_area for a screen of text (colors) */
uint8_t full_screen_color[DISP_CHAR_COLS * DISP_CHAR_ROWS];

/* Buffer to render 1 row of text into */
rgb_t __ili9341_text_row_buf[(DISP_CHAR_COLS * FONT_WIDTH) * FONT_HIEGHT];

/*! \brief Get the Color24 (RGB) value for a Color16 number. */
uint32_t color24_from_color16(color16_t c16) {
    return (_color16_map[c16 & 0x0f]);
}

scr_position_t cursor_get(void) {
    return __cursor_pos;
}

void cursor_home(void) {
    __cursor_pos.line = __margin_top;
    __cursor_pos.column = __margin_left;
}

void cursor_set(unsigned short line, unsigned short col) {
    if (line > DISP_CHAR_ROWS || col > DISP_CHAR_COLS) {
        return;
    }
    __cursor_pos = (scr_position_t){line, col};
}

void cursor_set_sp(scr_position_t pos) {
    if (pos.line > DISP_CHAR_ROWS || pos.column > DISP_CHAR_COLS) {
        return;
    }
    __cursor_pos = pos;
}

/*! \brief Create a `rgb_t` from a Color24 (RGB) value. */
rgb_t rgb_from_color24(uint32_t color24) {
    rgb_t rgb;
    rgb.r = (color24 & C24_MASK_RED) >> C24_SHIFT_RED;
    rgb.g = (color24 & C24_MASK_GREEN) >> C24_SHIFT_GREEN;
    rgb.b = (color24 & C24_MASK_BLUE) >> C24_SHIFT_BLUE;

    return (rgb);
}

/*! \brief Create 'color-byte number' from forground & background color numbers */
uint8_t colorbyte(uint8_t fg, uint8_t bg) {
    return ((bg << 4) | fg);
}

/*! \brief Get forground color number from color-byte number */
uint8_t fg_from_cb(uint8_t cb) {
    return (cb & 0x0f);
}

/*! \brief Get background color number from color-byte number */
uint8_t bg_from_cb(uint8_t cb) {
    return ((cb & 0xf0) >> 4);
}

/*! \brief Fill an RGB buffer with an RGB value. */
void fill_rgb_buf(rgb_t *buf, rgb_t rgb, size_t bufsize) {
    for (int i = 0; i < bufsize; i++) {
        *buf++ = rgb;
    }
}

#define DISP_BUF_SIZE (DISP_CHAR_COLS * CHAR_WIDTH) * CHAR_HIEGHT * sizeof(rgb_t) // 1 row of RGB characters

/*! \brief Initialize the display
 *  \ingroup display
 *
 * This must be called before using the display.
 */
void disp_init(void) {
    // run through the complete initialization process
    ili9341_spi_init();
    ili9341_disp_info_t *disp_info = ili9341_spi_info();

    // If in debug mode, print info about the display...
    if (option_value(OPTION_DEBUG)) {
        printf("Display MFG:         %02hhx\n", disp_info->lcd_mfg_id);
        printf("Display Ver:         %02hhx\n", disp_info->lcd_version);
        printf("Display ID:          %02hhx\n", disp_info->lcd_mfg_id);
        printf("Display Status 1:    %02hhx\n", disp_info->status1);
        printf("Display Status 2:    %02hhx\n", disp_info->status2);
        printf("Display Status 3:    %02hhx\n", disp_info->status3);
        printf("Display Status 4:    %02hhx\n", disp_info->status4);
        printf("Display PWR Mode:    %02hhx\n", disp_info->pwr_mode);
        printf("Display MADCTL:      %02hhx\n", disp_info->madctl);
        printf("Display Pixel Fmt:   %02hhx\n", disp_info->pixelfmt);
        printf("Display Image Fmt:   %02hhx\n", disp_info->imagefmt);
        printf("Display Signal Mode: %02hhx\n", disp_info->signal_mode);
        printf("Display Selftest:    %02hhx\n", disp_info->selftest);
    }

    __bg = C16_BLACK;
    __fg = C16_WHITE;
    disp_clear(true);
    margins_set(0, 0, DISP_CHAR_ROWS - 1, DISP_CHAR_COLS - 1);
}

/*
 * Clear the current text content and the screen.
*/
void disp_clear(bool paint) {
    memset(full_screen_text, 0x00, sizeof(full_screen_text));
    memset(full_screen_color, 0x00, sizeof(full_screen_color));
    memset(__dirty_text_rows, 0, sizeof(__dirty_text_rows));
    if (paint) {
        ili9341_screen_clr();
    }
}

void disp_char(unsigned short int row, unsigned short int col, char c, bool paint) {
    disp_char_colorbyte(row, col, c, colorbyte(__fg, __bg), paint);
}

void disp_char_color(unsigned short int row, unsigned short int col, char c, uint8_t fg, uint8_t bg, bool paint) {
    disp_char_colorbyte(row, col, c, colorbyte(fg, bg), paint);
}

/*
 * Display an ASCII character (plus some special characters)
 * If the top bit is set (c>127) the character is inverse (black on white background).
 */
void disp_char_colorbyte(unsigned short int row, unsigned short int col, char c, uint8_t color, bool paint) {
    if (row >= DISP_CHAR_ROWS || col >= DISP_CHAR_COLS) {
        return;  // Invalid row or column
    }
    *(full_screen_text + (row * DISP_CHAR_COLS) + col) = c;
    *(full_screen_color + (row * DISP_CHAR_COLS) + col) = color;
    unsigned char cl = c & 0x7F;
    if (paint) {
        // Actually render the characher glyph onto the screen.
        bool inverse = c & DISP_CHAR_INVERT_BIT;
        uint8_t fg = (inverse ? bg_from_cb(color) : fg_from_cb(color));
        uint8_t bg = (inverse ? fg_from_cb(color) : bg_from_cb(color));
        rgb_t fgrgb = rgb_from_color24(color24_from_color16(fg));
        rgb_t buf[CHAR_WIDTH * CHAR_HIEGHT];
        rgb_t *pbuf = buf;
        int glyphindex = (cl * CHAR_HIEGHT);
        fill_rgb_buf(buf, rgb_from_color24(color24_from_color16(bg)), (CHAR_WIDTH * CHAR_HIEGHT));
        for (int r = 0; r < CHAR_HIEGHT; r++) {
            // Get the glyph row for the character.
            uint16_t cgr = Font_Table[glyphindex++];
            uint16_t mask = 1 << (CHAR_WIDTH - 1);
            for (int c = 0; c < CHAR_WIDTH; c++) {
                // For each glyph column bit that is set, set the forground color in the character buffer.
                if (cgr & mask) {
                    *pbuf = fgrgb;
                }
                mask >>= 1;
                pbuf++;
            }
        }
        // Set a window into the right part of the screen
        unsigned short int x = col * CHAR_WIDTH;
        unsigned short int y = row * CHAR_HIEGHT;
        unsigned short int w = CHAR_WIDTH;
        unsigned short int h = CHAR_HIEGHT;
        ili9341_set_window(x, y, w, h);
        ili9341_screen_paint(buf, (CHAR_WIDTH * CHAR_HIEGHT)); 
    }
    else {
        __dirty_text_rows[row] = true;
    }
}

/*
 * Display all of the font characters. Wrap the characters until the full 
 * screen is filled.
 */
void disp_font_test(void) {
    disp_clear(true);
    // test font display 1
    char c = 0;
    for (unsigned int row = 0; row < DISP_CHAR_ROWS; row++) {
        for (unsigned int col = 0; col < DISP_CHAR_COLS; col++) {
            disp_char(row, col, c, true);
            c++;
        }
    }
}

/*
 * Paint the physical screen from the text.
 */ 
void disp_paint(void) {
    for (unsigned short int row = 0; row < DISP_CHAR_ROWS; row++) {
        if (__dirty_text_rows[row]) {
            disp_row_paint(row);
            __dirty_text_rows[row] = false; // The text row has been painted, mark it 'not dirty'
        }
    }
}

/*
 * Clear a row of text.
 */
void disp_row_clear(unsigned short int row, bool paint) {
    if (row >= DISP_CHAR_ROWS) {
        return;  // Invalid row or column
    }
    memset((full_screen_text + (row * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    memset((full_screen_color + (row * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    if (paint) {
        disp_row_paint(row);
    }
    else {
        __dirty_text_rows[row] = true;
    }
}

/*
 * Update the portion of the screen containing the given character row.
 */
void disp_row_paint(unsigned short int row) {
    if (row >= DISP_CHAR_ROWS) {
        return; // invalid row number
    }
    // Need to render this row.
    uint16_t scrnline = row * FONT_HIEGHT;
    rgb_t *rbuf = __ili9341_text_row_buf;
    for (int fontline = 0; fontline < FONT_HIEGHT; fontline++) {
        for (int textcol = 0; textcol < DISP_CHAR_COLS; textcol++) {
            int index = (row * DISP_CHAR_COLS) + textcol;
            unsigned char c = full_screen_text[index];
            bool invert = (c & DISP_CHAR_INVERT_BIT);
            c = c & DISP_CHAR_NORMAL_MASK;
            uint8_t color = full_screen_color[index];
            rgb_t fgrgb = rgb_from_color24(color24_from_color16(fg_from_cb(color)));
            rgb_t bgrgb = rgb_from_color24(color24_from_color16(bg_from_cb(color)));
            if (invert) {
                rgb_t tmp = fgrgb;
                fgrgb = bgrgb;
                bgrgb = tmp;
            }
            uint16_t glyphline = *(Font_Table + (c * FONT_HIEGHT) + fontline);
            for (uint16_t mask = (1 << (FONT_WIDTH-1)); mask; mask >>= 1) {
                if (glyphline & mask) {
                    // set the fg color
                    *rbuf++ = fgrgb;
                }
                else {
                    // set the bg color
                    *rbuf++ = bgrgb;
                }
            }
        }
    }
    // Write the pixel screen row to the display
    ili9341_set_window(0, scrnline, DISP_CHAR_COLS * FONT_WIDTH, FONT_HIEGHT);
    ili9341_screen_paint(__ili9341_text_row_buf, DISP_CHAR_COLS * FONT_WIDTH * FONT_HIEGHT);
}

/*
 * Scroll 2 or more rows up.
 */
void disp_rows_scroll_up(unsigned short int row_t, unsigned short int row_b, bool paint) {
    if (row_b <= row_t || row_b >= DISP_CHAR_ROWS) {
        return;  // Invalid row value
    }
    memmove((full_screen_text + (row_t * DISP_CHAR_COLS)), (full_screen_text + ((row_t + 1) * DISP_CHAR_COLS)), (row_b - row_t) * DISP_CHAR_COLS);
    memmove((full_screen_color + (row_t * DISP_CHAR_COLS)), (full_screen_color + ((row_t + 1) * DISP_CHAR_COLS)), (row_b - row_t) * DISP_CHAR_COLS);
    memset((full_screen_text + (row_b * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    memset((full_screen_color + (row_b * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    disp_update(paint);
}

/*
*/
void disp_set_text_colors(uint8_t fg, uint8_t bg) {
    // force them to 0-15
    __fg = fg & 0x0f;
    __bg = bg & 0x0f;
}

/*
 */
void disp_string(unsigned short int row, unsigned short int col, const char *pString, bool invert, bool paint) {
    if (row >= DISP_CHAR_ROWS || col >= DISP_CHAR_COLS) {
        return;  // Invalid row or column
    }
    for (unsigned char c = *pString; c != 0; c = *(++pString)) {
        if (invert) {
            c = c ^ DISP_CHAR_INVERT_BIT;
        }
        disp_char(row, col, c, false);
        col++;
        if (col == DISP_CHAR_COLS) {
            col = 0;
            row++;
            if (row == DISP_CHAR_ROWS) {
                break;  // ZZZ option to scroll
            }
        }
    }
    if (paint) {
        disp_paint();
    }
}

/*
*/
void disp_update(bool paint) {
    memset(__dirty_text_rows, true, sizeof(__dirty_text_rows));
    if (paint) {
        disp_paint();
    }
}

void margins_set(unsigned short top, unsigned short left, unsigned short bottom, unsigned short right) {
    if (left > DISP_CHAR_COLS || right > DISP_CHAR_COLS || left > right 
        || top > DISP_CHAR_ROWS || bottom > DISP_CHAR_ROWS || top > bottom) {
        return;
    }
    __margin_left = left;
    __margin_right = right;
    __margin_top = top;
    __margin_bottom = bottom;
}

void _crlf(short add_lines, bool paint) {
    short scroll_lines = add_lines;
    if (scroll_lines > __margin_bottom - __margin_top) {
        scroll_lines = __margin_bottom - __margin_top;
    }
    __cursor_pos.line++;
    if (__cursor_pos.line > __margin_bottom) {
        scroll_lines++;
        __cursor_pos.line = __margin_bottom;
    }
    __cursor_pos.column = __margin_left;
    if (scroll_lines) {
        short to_index = (__margin_top * DISP_CHAR_COLS) + __margin_left;
        short from_index = ((__margin_top + scroll_lines) * DISP_CHAR_COLS) + __margin_left;
        short lines = (__margin_bottom - __margin_top) - (scroll_lines - 1);
        for (short l = 0; l < lines; l++) {
            for (short c = 0; c < ((__margin_right - __margin_left) + 1); c++) {
                unsigned char d = full_screen_text[from_index + c];
                uint8_t colors = full_screen_color[from_index + c];
                full_screen_text[to_index + c] = d;
                full_screen_color[to_index + c] = colors;
            }
            __dirty_text_rows[__margin_top + l] = true;
            to_index += DISP_CHAR_COLS;
            from_index += DISP_CHAR_COLS;
        }
    }
    // blank out the cursor line
    short index_base = (__cursor_pos.line * DISP_CHAR_COLS) + __margin_left;
    uint8_t colors = colorbyte(__fg, __bg);
    __dirty_text_rows[__cursor_pos.line] = true;
    for (short i = 0; i < ((__margin_right - __margin_left) + 1); i++) {
        full_screen_text[index_base + i] = 0x00;
        full_screen_color[index_base + i] = colors;
    }
    if (paint) {
        // ZZZ for now, paint the dirty lines (full screen width).
        // ZZZ performance improvement would be to just paint the window.
        disp_paint();
    }
}

void printc(char c, bool paint) {
    // Since the line doesn't wrap right when the last column is written to,
    // and also, for the margins to get set between print operations, we need 
    // to check the current cursor position against the current margins and 
    // wrap/scroll if needed before printing the character.
    //
    // Also, this allows printing a NL ('\n') character, so nothing special is
    // done for it.
    //
    if (__cursor_pos.column < __margin_left) {
        __cursor_pos.column = __margin_left;
    }
    if (__cursor_pos.line < __margin_top) {
        __cursor_pos.line = __margin_top;
    }
    unsigned short scroll_lines = 0;
    if (__cursor_pos.line > __margin_bottom) {
        scroll_lines = __cursor_pos.line - __margin_bottom;
        __cursor_pos.line = __margin_bottom;
    }
    if (__cursor_pos.column > __margin_right) {
        _crlf(scroll_lines, paint);
    }
    disp_char(__cursor_pos.line, __cursor_pos.column, c, paint);
    __cursor_pos.column++;
}

void prints(char* str, bool paint) {
    unsigned char c;
    while ((c = *str++) != 0) {
        if (c == '\n') {
            _crlf(0, false);
        }
        else {
            printc(c, false);
        }
    }
    if (paint) {
        disp_paint();
    }
}
