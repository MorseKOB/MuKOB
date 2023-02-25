/**
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdlib.h>
#include "system_defs.h"
#include "mukboard.h"
#include "display_ili9341.h"
#include "display_i.h"
#include "font.h"
#include "font_10_16.h"
#include "string.h"

static void _disp_char(uint16_t aline, uint16_t col, char c, paint_control_t paint);
static void _disp_char_colorbyte(uint16_t aline, uint16_t col, char c, uint8_t color, paint_control_t paint);
static void _disp_line_clear(uint16_t aline, paint_control_t paint);
static void _disp_line_paint(uint16_t aline);
static void _fill_rgb16_buf(rgb16_t* buf, rgb16_t rgb16, size_t bufsize);
static uint16_t _translate_cursor_line(uint16_t curline);
static uint16_t _translate_line(uint16_t line);

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

/** @brief The current/active screen context */
static scr_context_t* _scr_ctx = NULL;

// ======================================================================================
// Internal functions
// ======================================================================================

/**
 * NOTE: This does not perform text line translation, nor bounds check.
 */
static void _disp_char(uint16_t line, uint16_t col, char c, paint_control_t paint) {
    _disp_char_colorbyte(line, col, c, colorbyte(_scr_ctx->color_fg_default, _scr_ctx->color_bg_default), paint);
}

/*
 * Display an ASCII character (plus some special characters)
 * If the top bit is set (c>127) the character is inverse (black on white background).
 *
 * Getting the glyph data for the font character is similar to `disp_line_paint`,
 * but the difference is that here we are rendering one single complete charcter.
 *
 * NOTE: This does not perform text line translation, nor bounds check.
 *
 */
static void _disp_char_colorbyte(uint16_t aline, uint16_t col, char c, uint8_t color, paint_control_t paint) {
    *(_scr_ctx->full_screen_text + (aline * _scr_ctx->cols) + col) = c;
    *(_scr_ctx->full_screen_color + (aline * _scr_ctx->cols) + col) = color;
    if (paint) {
        // Actually render the characher glyph onto the screen.
        bool invert = c & DISP_CHAR_INVERT_BIT;
        unsigned char cl = c & 0x7F;
        uint8_t fg = (invert ? bg_from_cb(color) : fg_from_cb(color));
        uint8_t bg = (invert ? fg_from_cb(color) : bg_from_cb(color));
        rgb16_t fgrgb = rgb16_from_color16(fg);
        rgb16_t bgrgb = rgb16_from_color16(bg);
        const font_info_t* fi = _scr_ctx->font_info;
        int8_t font_height = fi->height;
        int8_t font_width = fi->width;
        int8_t bpgl = fi->bytes_per_glyph_line;
        rgb16_t* rbuf = _scr_ctx->render_buf;
        uint16_t glyphindex = (cl * font_height * bpgl);
        // See if we need to show the cursor
        bool show_a_cursor = (_scr_ctx->show_cursor && col == _scr_ctx->cursor_pos.column && aline == _translate_cursor_line(_scr_ctx->cursor_pos.line));
        rgb16_t cursor_color = _scr_ctx->cursor_color;
        for (int glyph_line = 0; glyph_line < font_height; glyph_line++) {
            if (show_a_cursor && glyph_line == _scr_ctx->font_info->suggested_cursor_line) {
                for (int c = 0; c < font_width; c++) {
                    *rbuf++ = cursor_color;
                }
            }
            else {
                // Get each glyph row for the character (each line of the font char height).
                uint32_t cgr = 0;
                uint32_t mask = 1u << (font_width - 1u);
                for (int byte = 0; byte < bpgl; byte++) {
                    cgr |= (fi->glyphs[glyphindex + byte + (glyph_line * bpgl)]) << (8u * byte);
                }
                for (int c = 0; c < font_width; c++) {
                    // For each glyph column bit that is set, set the forground color in the character buffer.
                    if (cgr & mask) {
                        *rbuf = fgrgb;
                    }
                    else {
                        *rbuf = bgrgb;
                    }
                    mask >>= 1u;
                    rbuf++;
                }
            }
        }
        // Set a window into the right part of the screen
        uint16_t x = col * font_width;
        uint16_t y = aline * font_height;
        uint16_t w = font_width;
        uint16_t h = font_height;
        ili9341_window_set_area(x, y, w, h);
        ili9341_screen_paint(_scr_ctx->render_buf, (font_width * font_height));
    }
    else {
        _scr_ctx->dirty_text_lines[aline] = true;
    }
}

/*
 * Clear a line of text.
 *
 * NOTE: This does not perform text line translation, nor bounds check.
 */
static void _disp_line_clear(uint16_t aline, paint_control_t paint) {
    memset((_scr_ctx->full_screen_text + (aline * _scr_ctx->cols)), SPACE_CHR, _scr_ctx->cols);
    memset((_scr_ctx->full_screen_color + (aline * _scr_ctx->cols)), colorbyte(_scr_ctx->color_fg_default, _scr_ctx->color_bg_default), _scr_ctx->cols);
    if (paint) {
        _disp_line_paint(aline);
    }
    else {
        _scr_ctx->dirty_text_lines[aline] = true;
    }
}

/*
 * Update the portion of the screen containing the given character line.
 *
 * Getting the glyph data for the font character is similar to `disp_char_colorbyte`,
 * but the difference is that here we are rendering a complete line of characters one
 * glyph line at a time, and repeating that for all of the glyph lines.
 *
 * NOTE: This does not perform text line translation, nor bounds check.
 */
static void _disp_line_paint(uint16_t aline) {
    const font_info_t* fi = _scr_ctx->font_info;
    int8_t font_height = fi->height;
    int8_t font_width = fi->width;
    int8_t bpgl = fi->bytes_per_glyph_line;
    bool show_cursor = (_scr_ctx->show_cursor && aline == _translate_cursor_line(_scr_ctx->cursor_pos.line));
    int8_t cursor_show_row = fi->suggested_cursor_line;
    uint16_t screen_line = aline * font_height;
    rgb16_t* rbuf = _scr_ctx->render_buf;
    for (int glyph_line = 0; glyph_line < font_height; glyph_line++) {
        for (uint16_t textcol = 0; textcol < _scr_ctx->cols; textcol++) {
            uint16_t index = (aline * _scr_ctx->cols) + textcol;
            unsigned char c = _scr_ctx->full_screen_text[index];
            bool invert = c & DISP_CHAR_INVERT_BIT;
            unsigned char cl = c & 0x7F;
            uint8_t color = _scr_ctx->full_screen_color[index];
            uint8_t fg = (invert ? bg_from_cb(color) : fg_from_cb(color));
            uint8_t bg = (invert ? fg_from_cb(color) : bg_from_cb(color));
            rgb16_t fgrgb = rgb16_from_color16(fg);
            rgb16_t bgrgb = rgb16_from_color16(bg);
            // Get the glyph row for the character (a line of the font char height).
            uint16_t glyphindex = (cl * font_height * bpgl);
            uint32_t cgr = 0;
            for (int byte = 0; byte < bpgl; byte++) {
                cgr |= (fi->glyphs[glyphindex + byte + (glyph_line * bpgl)]) << (8u * byte);
            }
            for (uint32_t mask = (1u << (font_width - 1u)); mask; mask >>= 1u) {
                if (show_cursor && textcol == _scr_ctx->cursor_pos.column && glyph_line == cursor_show_row) {
                    // Draw a cursor line
                    *rbuf++ = _scr_ctx->cursor_color;
                }
                else if (cgr & mask) {
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
    ili9341_window_set_area(0, screen_line, _scr_ctx->cols * font_width, font_height);
    ili9341_screen_paint(_scr_ctx->render_buf, _scr_ctx->cols * font_width * font_height);
}

/*! @brief Fill an RGB-16 buffer with an RGB-16 value. */
static void _fill_rgb16_buf(rgb16_t* buf, rgb16_t rgb16, size_t bufsize) {
    for (int i = 0; i < bufsize; i++) {
        *buf++ = rgb16;
    }
}

/**
 * @brief Get the absolute text line index for the current cursor position, accounting for scroll.
 *
 * @return uint16_t The absolute text line number for the cursor
 */
static uint16_t _translate_cursor_line(uint16_t curline) {
    // Cursor line is relative to the scroll area. For example, cursor line 1 is the
    // 2nd line within the scroll area.
    uint16_t aline = curline + _scr_ctx->scroll_start;
    if (aline >= _scr_ctx->lines - _scr_ctx->fixed_area_bottom_size) {
        aline -= _scr_ctx->scroll_size;
    }

    return (aline);
}

/**
 * @brief Get the absolute text line index for the requested line, accounting for scroll.
 *
 * @return uint16_t The absolute text line number
 */
static uint16_t _translate_line(uint16_t line) {
    // A bit more work is needed than with the cursor, since `line` can be outside
    // of the scroll area and is from the top of the screen (absolute line 0).
    if (line < _scr_ctx->fixed_area_top_size || line >= (_scr_ctx->lines - _scr_ctx->fixed_area_bottom_size)) {
        return (line);
    }
    // The line is within the scroll area, so adjust it and translate it the same as the cursor.
    line -= _scr_ctx->fixed_area_top_size;
    return (_translate_cursor_line(line));
}

// ======================================================================================
// Public functions
// ======================================================================================

inline rgb16_t rgb16_from_color16(colorn16_t c16) {
    return (_color16_map[c16 & 0x0f]);
}

inline scr_position_t cursor_get(void) {
    return _scr_ctx->cursor_pos;
}

void cursor_home(void) {
    cursor_set(0, 0);
}

void cursor_show(bool show) {
    _scr_ctx->show_cursor = show;
}

void cursor_set(uint16_t line, uint16_t col) {
    cursor_set_sp((scr_position_t){line, col});
}

void cursor_set_sp(scr_position_t pos) {
    if (pos.line >= _scr_ctx->scroll_size || pos.column >= _scr_ctx->cols) {
        return;
    }
    _scr_ctx->cursor_pos = pos;
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

/*
 * Clear the current text content and the screen.
*/
void disp_clear(paint_control_t paint) {
    size_t chars = _scr_ctx->lines * _scr_ctx->cols;
    memset(_scr_ctx->full_screen_text, SPACE_CHR, chars);
    memset(_scr_ctx->full_screen_color, colorbyte(_scr_ctx->color_fg_default, _scr_ctx->color_bg_default), chars);
    memset(_scr_ctx->dirty_text_lines, false, _scr_ctx->lines);
    cursor_home();
    if (paint) {
        display_backlight_on(false);    // Turning off the backlight helps this from being distracting
        ili9341_screen_clr(_scr_ctx->color_bg_default, false);
        display_backlight_on(true);
    }
}

void disp_char(uint16_t line, uint16_t col, char c, paint_control_t paint) {
    if (line >= _scr_ctx->lines || col >= _scr_ctx->cols) {
        return;  // Invalid line or column
    }
    uint16_t aline = _translate_line(line); // Adjust for scroll
    _disp_char(aline, col, c, paint);
}

void disp_char_color(uint16_t line, uint16_t col, char c, colorn16_t fg, colorn16_t bg, paint_control_t paint) {
    disp_char_colorbyte(line, col, c, colorbyte(fg, bg), paint);
}

/*
 * Display an ASCII character (plus some special characters)
 * If the top bit is set (c>127) the character is inverse (black on white background).
 *
 * Getting the glyph data for the font character is similar to `disp_line_paint`,
 * but the difference is that here we are rendering one single complete charcter.
 *
 */
void disp_char_colorbyte(uint16_t line, uint16_t col, char c, colorbyte_t color, paint_control_t paint) {
    if (line >= _scr_ctx->lines || col >= _scr_ctx->cols) {
        return;  // Invalid line or column
    }
    uint16_t aline = _translate_line(line); // Adjust for scroll
    _disp_char_colorbyte(aline, col, c, color, paint);
}

/*
 * Display all of the font characters. Wrap the characters until the full
 * screen is filled.
 */
void disp_font_test(void) {
    disp_clear(true);
    // test font display 1
    char c = 0;
    int16_t lines = _scr_ctx->lines;
    int16_t cols = _scr_ctx->cols;
    for (unsigned int line = 0; line < lines; line++) {
        for (unsigned int col = 0; col < cols; col++) {
            _disp_char(line, col, c, true);
            c++;
        }
    }
}

uint16_t disp_info_columns() {
    return (_scr_ctx->cols);
}

uint16_t disp_info_lines() {
    return (_scr_ctx->lines);
}

uint16_t disp_info_fixed_top_lines() {
    return (_scr_ctx->fixed_area_top_size);
}

uint16_t disp_info_fixed_bottom_lines() {
    return (_scr_ctx->fixed_area_bottom_size);
}

uint16_t disp_info_scroll_lines() {
    return (_scr_ctx->scroll_size);
}

/*! @brief Initialize the display
 *  \ingroup display
 *
 * This must be called before using the display, but should only be called once.
 */
void disp_init(void) {
    // run through the complete initialization process

    ili9341_init();
    ili9341_disp_info_t* disp_info = ili9341_info();

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

    if (_scr_ctx != NULL) {
        warn_printf("`disp_init` called multiple times!\n");
        return;
    }

    screen_new();
}

/*
 * Paint the physical screen from the text.
 */
void disp_paint(void) {
    int16_t lines = _scr_ctx->lines;
    uint16_t aline;
    bool some_lines_dirty = false;
    for (uint16_t i = 0; i < lines && !some_lines_dirty; i++) {
        some_lines_dirty |= _scr_ctx->dirty_text_lines[i];
    }
    if (some_lines_dirty) {
        for (uint16_t line = 0; line < lines; line++) {
            aline = _translate_line(line);
            if (_scr_ctx->dirty_text_lines[aline]) {
                _disp_line_paint(aline);
                _scr_ctx->dirty_text_lines[aline] = false; // The text line has been painted, mark it 'not dirty'
            }
        }
    }
}

/*
 * Clear a line of text.
 */
void disp_line_clear(uint16_t line, paint_control_t paint) {
    if (line >= _scr_ctx->lines) {
        return;  // Invalid line or column
    }
    uint16_t aline = _translate_line(line); // Adjust for scroll
    _disp_line_clear(aline, paint);
}

/*
 * Update the portion of the screen containing the given character line.
 *
 * Getting the glyph data for the font character is similar to `disp_char_colorbyte`,
 * but the difference is that here we are rendering a complete line of characters one
 * glyph line at a time, and repeating that for all of the glyph lines.
 */
void disp_line_paint(uint16_t line) {
    if (line >= _scr_ctx->lines) {
        return; // invalid line number
    }
    line = _translate_line(line);
    _disp_line_paint(line);
}

void disp_set_text_colors(colorn16_t fg, colorn16_t bg) {
    // force them to 0-15
    _scr_ctx->color_fg_default = fg & 0x0f;
    _scr_ctx->color_bg_default = bg & 0x0f;
}

void disp_string(uint16_t line, uint16_t col, const char *pString, bool invert, paint_control_t paint) {
    if (line >= _scr_ctx->lines || col >= _scr_ctx->cols) {
        return;  // Invalid line or column
    }
    for (unsigned char c = *pString; c != 0; c = *(++pString)) {
        if (invert) {
            c = c ^ DISP_CHAR_INVERT_BIT;
        }
        disp_char(line, col, c, false);
        col++;
        if (col == _scr_ctx->cols) {
            col = 0;
            line++;
            if (line == _scr_ctx->lines) {
                line = 0;
            }
        }
    }
    if (paint) {
        disp_paint();
    }
}

void disp_update(paint_control_t paint) {
    // Mark all lines as 'dirty' so they will be re-rendered during a `paint` operation.
    memset(_scr_ctx->dirty_text_lines, true, _scr_ctx->lines * sizeof(bool));
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

void print_crlf(int16_t add_lines, paint_control_t paint) {
    int16_t total_scroll_lines = add_lines;
    uint16_t ss = _scr_ctx->scroll_start;  // Scroll start line
    uint16_t scroll_lines = _scr_ctx->scroll_size;  // Screen scroll lines
    uint16_t scroll_cap = _scr_ctx->lines - _scr_ctx->fixed_area_bottom_size - 1;
    uint16_t cursor_cap = scroll_lines - 1;
    scr_position_t new_cp = { _scr_ctx->cursor_pos.line + 1, 0 };
    uint16_t aline;
    if (new_cp.line > cursor_cap) {
        total_scroll_lines += (new_cp.line - cursor_cap);
        new_cp.line = cursor_cap;
    }
    if (total_scroll_lines > scroll_lines) {
        total_scroll_lines = scroll_lines;
    }
    if (total_scroll_lines > 0) {
        // Advance the Scroll-Start
        ss++;
        if (ss > scroll_cap) {
            ss = _scr_ctx->fixed_area_top_size;
        }
        _scr_ctx->scroll_start = ss;
        // blank out what will be the cursor line
        aline = _translate_cursor_line(new_cp.line);
        _disp_line_clear(aline, paint);
        ili9341_scroll_set_start(ss * _scr_ctx->font_info->height);
    }
    else {
        // blank out what will be the cursor line
        aline = _translate_cursor_line(new_cp.line);
        _disp_line_clear(aline, paint);
    }
    _scr_ctx->cursor_pos = new_cp;
}

void printc(char c, paint_control_t paint) {
    // Since the line doesn't wrap right when the last column is written to,
    // we need to check the current cursor position against the current margins and
    // wrap/scroll if needed before printing the character.
    //
    // Also, this allows printing a NL ('\n') character, so nothing special is
    // done for it.
    //
    if (_scr_ctx->cursor_pos.column >= _scr_ctx->cols) {
        print_crlf(0, paint);
    }
    uint16_t aline = _translate_cursor_line(_scr_ctx->cursor_pos.line);
    _disp_char(aline, _scr_ctx->cursor_pos.column++, c, paint);
}

void prints(char* str, paint_control_t paint) {
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

void screen_close() {
    if (!_has_scr_context()) {
        warn_printf("Display - Trying to close main screen context. Ignoring `screen_close()` call.");
        return;
    }
    // Free the buffers from the current context...
    free(_scr_ctx->full_screen_text);
    free(_scr_ctx->full_screen_color);
    free(_scr_ctx->dirty_text_lines);
    free(_scr_ctx->render_buf);
    // Now free the current context
    free(_scr_ctx);

    // Get the top context and make it current
    _scr_ctx = _pop_scr_context();
    // This will re-configure the ILI9341 for correct scrolling
    scroll_area_define(_scr_ctx->fixed_area_top_size, _scr_ctx->fixed_area_bottom_size);
    disp_update(Paint);
}

bool screen_new() {
    // First thing is to see if we can push the current screen context if there is one
    if (_scr_ctx) {
        if (!_push_scr_context(_scr_ctx)) {
            return false;
        }
    }
    scr_context_t* scr_context = malloc(sizeof(scr_context_t));
    if (scr_context == NULL) {
        error_printf("Display - Could not allocate a screen context.");
        return false;
    }
    // For now, we only have one font - get its info
    const font_info_t* fi = &font_10_16;
    info_printf("Display font: %s.\n", fi->name);
    scr_context->font_info = fi;
    scr_context->color_bg_default = C16_BLACK;
    scr_context->color_fg_default = C16_WHITE;
    // Figure out how many lines and columns we have
    int16_t cols = ILI9341_WIDTH / fi->width;
    int16_t lines = ILI9341_HEIGHT / fi->height;
    info_printf("Display size: %hdx%hd (cols x lines)\n", cols, lines);
    size_t chars = lines * cols;
    scr_context->cols = cols;
    scr_context->lines = lines;
    // Allocate buffers
    scr_context->full_screen_text = (uint8_t*)malloc(chars);
    scr_context->full_screen_color = (colorbyte_t*)malloc(chars);
    scr_context->dirty_text_lines = (bool*)calloc(lines, sizeof(bool));
    scr_context->render_buf = (rgb16_t*)malloc(fi->width * fi->height * cols * sizeof(rgb16_t));
    // Default scroll area to the full screen
    scr_context->fixed_area_top_size = 0;
    scr_context->fixed_area_bottom_size = 0;
    scr_context->scroll_size = lines;
    scr_context->scroll_start = 0;
    scr_context->cursor_pos = (scr_position_t){ 0, 0 };
    scr_context->show_cursor = false; // Start with the cursor off (typical for dialogs)
    // 16-bit from R5G6B5 = R5*2048 + G6*32 + B5
    scr_context->cursor_color = (rgb16_t)0x05A0;    // Custom Green so it doesn't match any of the 16 (0 45 0)
    // Set this as the current context before calling other 'screen' functions.
    _scr_ctx = scr_context;
    scroll_area_define(0, 0);   // This will configure the ILI9341 for scrolling
    disp_clear(Paint);

    return (true);
}

void scroll_area_define(uint16_t top_fixed_size, uint16_t bottom_fixed_size) {
    uint16_t screen_lines = _scr_ctx->lines;
    uint16_t fixed_lines = top_fixed_size + bottom_fixed_size;
    int16_t scroll_lines = screen_lines - fixed_lines;
    if (scroll_lines < 0) {
        error_printf("Display - Attempting to set fixed regions of screen larger than total screen lines.");
        return;
    }
    else if (scroll_lines == 0) {
        // To keep the `print...` functions working, while avoiding a bunch of special
        // case code, we treat this setting the same as full screen scroll.
        // If we find a use case that truly requires the whole screen to be fixed, 
        // we will address this.
        top_fixed_size = 0;
        bottom_fixed_size = 0;
    }
    _scr_ctx->scroll_start = top_fixed_size;
    _scr_ctx->fixed_area_top_size = top_fixed_size;
    _scr_ctx->fixed_area_bottom_size = bottom_fixed_size;
    _scr_ctx->scroll_size = screen_lines - (top_fixed_size + bottom_fixed_size);
    ili9341_scroll_set_area(top_fixed_size * _scr_ctx->font_info->height, bottom_fixed_size * _scr_ctx->font_info->height);
    ili9341_scroll_set_start(_scr_ctx->scroll_start);
    cursor_home();
}

