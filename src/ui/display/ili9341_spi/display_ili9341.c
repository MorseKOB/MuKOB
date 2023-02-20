/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "system_defs.h"
#include "mukboard.h"
#include "display_ili9341.h"
#include "display.h"
#include "font.h"
#include "string.h"

/*! @brief Map of Color24 (RGB) values indexed by Color16 numbers. */
rgb16_t _color16_map[] = {
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

/* Dirty text rows */
static bool _dirty_text_lines[DISP_CHAR_LINES];
/* Default forground color */
static uint8_t _fg;
/* Default background color */
static uint8_t _bg;

/* Cursor position */
static scr_position_t _cursor_pos;

/* Margin boundaries */
static unsigned int _margin_bottom;
static unsigned int _margin_left;
static unsigned int _margin_right;
static unsigned int _margin_top;

/** @brief Memory display_full_area for a screen of text (characters) */
uint8_t full_screen_text[DISP_CHAR_COLS * DISP_CHAR_LINES];
/** @brief Memory display_full_area for a screen of text (colors) */
colorbyte_t full_screen_color[DISP_CHAR_COLS * DISP_CHAR_LINES];

/** @brief Buffer to render 1 line of text into */
rgb16_t text_line_render_buf[(DISP_CHAR_COLS * FONT_WIDTH) * FONT_HEIGHT];

inline rgb16_t rgb16_from_color16(colorn16_t c16) {
    return (_color16_map[c16 & 0x0f]);
}

inline scr_position_t cursor_get(void) {
    return _cursor_pos;
}

void cursor_home(void) {
    _cursor_pos.line = _margin_top;
    _cursor_pos.column = _margin_left;
}

void cursor_set(unsigned short line, unsigned short col) {
    if (line > DISP_CHAR_LINES || col > DISP_CHAR_COLS) {
        return;
    }
    _cursor_pos = (scr_position_t){line, col};
}

void cursor_set_sp(scr_position_t pos) {
    if (pos.line > DISP_CHAR_LINES || pos.column > DISP_CHAR_COLS) {
        return;
    }
    _cursor_pos = pos;
}

/*! @brief Create 'color-byte number' from forground & background color numbers */
inline colorbyte_t colorbyte(colorn16_t fg, colorn16_t bg) {
    return ((bg << 4u) | fg);
}

/*! @brief Get forground color number from color-byte number */
inline colorn16_t fg_from_cb(colorbyte_t cb) {
    return (cb & 0x0f);
}

/*! @brief Get background color number from color-byte number */
inline colorn16_t bg_from_cb(colorbyte_t cb) {
    return ((cb & 0xf0) >> 4u);
}

/*! @brief Fill an RGB-16 buffer with an RGB-16 value. */
void fill_rgb16_buf(rgb16_t *buf, rgb16_t rgb16, size_t bufsize) {
    for (int i = 0; i < bufsize; i++) {
        *buf++ = rgb16;
    }
}

#define DISP_BUF_SIZE (DISP_CHAR_COLS * CHAR_WIDTH) * CHAR_HIEGHT * sizeof(rgb16_t) // 1 line of RGB-16 characters

/*! @brief Initialize the display
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
        debug_printf("Display MFG:         %02hhx\n", disp_info->lcd_mfg_id);
        debug_printf("Display Ver:         %02hhx\n", disp_info->lcd_version);
        debug_printf("Display ID:          %02hhx\n", disp_info->lcd_mfg_id);
        debug_printf("Display Status 1:    %02hhx\n", disp_info->status1);
        debug_printf("Display Status 2:    %02hhx\n", disp_info->status2);
        debug_printf("Display Status 3:    %02hhx\n", disp_info->status3);
        debug_printf("Display Status 4:    %02hhx\n", disp_info->status4);
        debug_printf("Display PWR Mode:    %02hhx\n", disp_info->pwr_mode);
        debug_printf("Display MADCTL:      %02hhx\n", disp_info->madctl);
        debug_printf("Display Pixel Fmt:   %02hhx\n", disp_info->pixelfmt);
        debug_printf("Display Image Fmt:   %02hhx\n", disp_info->imagefmt);
        debug_printf("Display Signal Mode: %02hhx\n", disp_info->signal_mode);
        debug_printf("Display Selftest:    %02hhx\n", disp_info->selftest);
    }

    _bg = C16_BLACK;
    _fg = C16_WHITE;
    disp_clear(true);
    margins_set(0, 0, DISP_CHAR_LINES - 1, DISP_CHAR_COLS - 1);
}

/*
 * Clear the current text content and the screen.
*/
void disp_clear(bool paint) {
    memset(full_screen_text, 0x20, sizeof(full_screen_text));
    memset(full_screen_color, 0x00, sizeof(full_screen_color));
    memset(_dirty_text_lines, 0, sizeof(_dirty_text_lines));
    if (paint) {
        ili9341_screen_clr(false);
    }
}

void disp_char(unsigned short int line, unsigned short int col, char c, bool paint) {
    disp_char_colorbyte(line, col, c, colorbyte(_fg, _bg), paint);
}

void disp_char_color(unsigned short int line, unsigned short int col, char c, uint8_t fg, uint8_t bg, bool paint) {
    disp_char_colorbyte(line, col, c, colorbyte(fg, bg), paint);
}

/*
 * Display an ASCII character (plus some special characters)
 * If the top bit is set (c>127) the character is inverse (black on white background).
 */
void disp_char_colorbyte(unsigned short int line, unsigned short int col, char c, uint8_t color, bool paint) {
    if (line >= DISP_CHAR_LINES || col >= DISP_CHAR_COLS) {
        return;  // Invalid line or column
    }
    *(full_screen_text + (line * DISP_CHAR_COLS) + col) = c;
    *(full_screen_color + (line * DISP_CHAR_COLS) + col) = color;
    unsigned char cl = c & 0x7F;
    if (paint) {
        // Actually render the characher glyph onto the screen.
        bool inverse = c & DISP_CHAR_INVERT_BIT;
        uint8_t fg = (inverse ? bg_from_cb(color) : fg_from_cb(color));
        uint8_t bg = (inverse ? fg_from_cb(color) : bg_from_cb(color));
        rgb16_t fgrgb = rgb16_from_color16(fg);
        rgb16_t buf[FONT_WIDTH * FONT_HEIGHT];
        rgb16_t *pbuf = buf;
        int glyphindex = (cl * FONT_HEIGHT);
        fill_rgb16_buf(buf, rgb16_from_color16(bg), (FONT_WIDTH * FONT_HEIGHT));
        for (int r = 0; r < FONT_HEIGHT; r++) {
            // Get the glyph row for the character.
            uint16_t cgr = Font_Table[glyphindex++];
            uint16_t mask = 1u << (FONT_WIDTH - 1);
            for (int c = 0; c < FONT_WIDTH; c++) {
                // For each glyph column bit that is set, set the forground color in the character buffer.
                if (cgr & mask) {
                    *pbuf = fgrgb;
                }
                mask >>= 1u;
                pbuf++;
            }
        }
        // Set a window into the right part of the screen
        unsigned short int x = col * FONT_WIDTH;
        unsigned short int y = line * FONT_HEIGHT;
        unsigned short int w = FONT_WIDTH;
        unsigned short int h = FONT_HEIGHT;
        ili9341_window_set_area(x, y, w, h);
        ili9341_screen_paint(buf, (FONT_WIDTH * FONT_HEIGHT));
    }
    else {
        _dirty_text_lines[line] = true;
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
    for (unsigned int line = 0; line < DISP_CHAR_LINES; line++) {
        for (unsigned int col = 0; col < DISP_CHAR_COLS; col++) {
            disp_char(line, col, c, true);
            c++;
        }
    }
}

/*
 * Paint the physical screen from the text.
 */
void disp_paint(void) {
    for (unsigned short int line = 0; line < DISP_CHAR_LINES; line++) {
        if (_dirty_text_lines[line]) {
            disp_line_paint(line);
            _dirty_text_lines[line] = false; // The text line has been painted, mark it 'not dirty'
        }
    }
}

/*
 * Clear a line of text.
 */
void disp_line_clear(unsigned short int line, bool paint) {
    if (line >= DISP_CHAR_LINES) {
        return;  // Invalid line or column
    }
    memset((full_screen_text + (line * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    memset((full_screen_color + (line * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    if (paint) {
        disp_line_paint(line);
    }
    else {
        _dirty_text_lines[line] = true;
    }
}

/*
 * Update the portion of the screen containing the given character line.
 */
void disp_line_paint(unsigned short int line) {
    if (line >= DISP_CHAR_LINES) {
        return; // invalid line number
    }
    // Need to render this line.
    uint16_t scrnline = line * FONT_HEIGHT;
    rgb16_t *rbuf = text_line_render_buf;
    for (int fontline = 0; fontline < FONT_HEIGHT; fontline++) {
        for (int textcol = 0; textcol < DISP_CHAR_COLS; textcol++) {
            int index = (line * DISP_CHAR_COLS) + textcol;
            unsigned char c = full_screen_text[index];
            bool invert = (c & DISP_CHAR_INVERT_BIT);
            c = c & DISP_CHAR_NORMAL_MASK;
            uint8_t color = full_screen_color[index];
            rgb16_t fgrgb = rgb16_from_color16(fg_from_cb(color));
            rgb16_t bgrgb = rgb16_from_color16(bg_from_cb(color));
            if (invert) {
                rgb16_t tmp = fgrgb;
                fgrgb = bgrgb;
                bgrgb = tmp;
            }
            uint16_t glyphline = *(Font_Table + (c * FONT_HEIGHT) + fontline);
            for (uint16_t mask = (1u << (FONT_WIDTH-1)); mask; mask >>= 1u) {
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
    ili9341_window_set_area(0, scrnline, DISP_CHAR_COLS * FONT_WIDTH, FONT_HEIGHT);
    ili9341_screen_paint(text_line_render_buf, DISP_CHAR_COLS * FONT_WIDTH * FONT_HEIGHT);
}

/*
 * Scroll 2 or more rows up.
 */
void disp_rows_scroll_up(unsigned short int line_t, unsigned short int line_b, bool paint) {
    if (line_b <= line_t || line_b >= DISP_CHAR_LINES) {
        return;  // Invalid line value
    }
    memmove((full_screen_text + (line_t * DISP_CHAR_COLS)), (full_screen_text + ((line_t + 1) * DISP_CHAR_COLS)), (line_b - line_t) * DISP_CHAR_COLS);
    memmove((full_screen_color + (line_t * DISP_CHAR_COLS)), (full_screen_color + ((line_t + 1) * DISP_CHAR_COLS)), (line_b - line_t) * DISP_CHAR_COLS);
    memset((full_screen_text + (line_b * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    memset((full_screen_color + (line_b * DISP_CHAR_COLS)), 0x00, DISP_CHAR_COLS);
    disp_update(paint);
}

void disp_set_text_colors(uint8_t fg, uint8_t bg) {
    // force them to 0-15
    _fg = fg & 0x0f;
    _bg = bg & 0x0f;
}

void disp_string(unsigned short int line, unsigned short int col, const char *pString, bool invert, bool paint) {
    if (line >= DISP_CHAR_LINES || col >= DISP_CHAR_COLS) {
        return;  // Invalid line or column
    }
    for (unsigned char c = *pString; c != 0; c = *(++pString)) {
        if (invert) {
            c = c ^ DISP_CHAR_INVERT_BIT;
        }
        disp_char(line, col, c, false);
        col++;
        if (col == DISP_CHAR_COLS) {
            col = 0;
            line++;
            if (line == DISP_CHAR_LINES) {
                break;  // ZZZ option to scroll
            }
        }
    }
    if (paint) {
        disp_paint();
    }
}

void disp_update(bool paint) {
    memset(_dirty_text_lines, true, sizeof(_dirty_text_lines));
    if (paint) {
        disp_paint();
    }
}

void disp_c16_color_chart() {
    disp_clear(true);
    // VGA Color Test
    disp_set_text_colors(C16_BR_WHITE, C16_BLACK);
    for (uint8_t i = 0; i < 8; i++) {
        char c = '0'+i;
        disp_char(4, (2 * i) + 5, c, true);
        disp_char_colorbyte(5, (2 * i) + 5, DISP_CHAR_INVERT_BIT | SPACE_CHR, i, true);
    }
    for (uint8_t i = 8; i < 16; i++) {
        char c = '0' + i;
        if (c > '9') c += 7;
        disp_char(7, (2 * (i - 8)) + 5, c, true);
        disp_char_colorbyte(8, (2 * (i - 8)) + 5 , DISP_CHAR_INVERT_BIT | SPACE_CHR, i, true);
    }
}

void margins_set(unsigned short top, unsigned short left, unsigned short bottom, unsigned short right) {
    if (left > DISP_CHAR_COLS || right > DISP_CHAR_COLS || left > right
        || top > DISP_CHAR_LINES || bottom > DISP_CHAR_LINES || top > bottom) {
        return;
    }
    _margin_left = left;
    _margin_right = right;
    _margin_top = top;
    _margin_bottom = bottom;
}

void print_crlf(short add_lines, bool paint) {
    short scroll_lines = add_lines;
    if (scroll_lines > _margin_bottom - _margin_top) {
        scroll_lines = _margin_bottom - _margin_top;
    }
    _cursor_pos.line++;
    if (_cursor_pos.line > _margin_bottom) {
        scroll_lines++;
        _cursor_pos.line = _margin_bottom;
    }
    _cursor_pos.column = _margin_left;
    if (scroll_lines) {
        short to_index = (_margin_top * DISP_CHAR_COLS) + _margin_left;
        short from_index = ((_margin_top + scroll_lines) * DISP_CHAR_COLS) + _margin_left;
        short lines = (_margin_bottom - _margin_top) - (scroll_lines - 1);
        for (short l = 0; l < lines; l++) {
            for (short c = 0; c < ((_margin_right - _margin_left) + 1); c++) {
                unsigned char d = full_screen_text[from_index + c];
                colorbyte_t colors = full_screen_color[from_index + c];
                full_screen_text[to_index + c] = d;
                full_screen_color[to_index + c] = colors;
            }
            _dirty_text_lines[_margin_top + l] = true;
            to_index += DISP_CHAR_COLS;
            from_index += DISP_CHAR_COLS;
        }
    }
    // blank out the cursor line
    short index_base = (_cursor_pos.line * DISP_CHAR_COLS) + _margin_left;
    colorbyte_t colors = colorbyte(_fg, _bg);
    _dirty_text_lines[_cursor_pos.line] = true;
    for (short i = 0; i < ((_margin_right - _margin_left) + 1); i++) {
        full_screen_text[index_base + i] = SPACE_CHR;
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
    // done for it. If
    //
    if (_cursor_pos.column < _margin_left) {
        _cursor_pos.column = _margin_left;
    }
    if (_cursor_pos.line < _margin_top) {
        _cursor_pos.line = _margin_top;
    }
    unsigned short scroll_lines = 0;
    if (_cursor_pos.line > _margin_bottom) {
        scroll_lines = _cursor_pos.line - _margin_bottom;
        _cursor_pos.line = _margin_bottom;
    }
    if (_cursor_pos.column > _margin_right) {
        print_crlf(scroll_lines, paint);
    }
    disp_char(_cursor_pos.line, _cursor_pos.column, c, paint);
    _cursor_pos.column++;
}

void prints(char* str, bool paint) {
    unsigned char c;
    while ((c = *str++) != 0) {
        if (c == '\n') {
            print_crlf(0, false);
        }
        else {
            printc(c, false);
        }
    }
    if (paint) {
        disp_paint();
    }
}
