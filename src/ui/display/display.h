/**
 * @brief Display functionality.
 * @ingroup display
 *
 * This defines the display/screen functionality in a generic way. The
 * `display_xxx` files contain implementations that are specific to a
 * particular display/screen device.
 *
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef DISPLAY_H
#define DISPLAY_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Bit to OR in to invert a character (display black char on white background) */
#define DISP_CHAR_INVERT_BIT 0x80
/** @brief Mask to AND with a character to remove invert (display white char on black background) */
#define DISP_CHAR_NORMAL_MASK 0x7F


/** @brief Bit to OR in to invert a character (display black char on white background) */
#define DISP_CHAR_INVERT_BIT 0x80
/** @brief Mask to AND with a character to remove invert (display white char on black background) */
#define DISP_CHAR_NORMAL_MASK 0x7F

/** @brief Red-5-bits Green-6-bits Blue-5-bits (16 bit unsigned) */
typedef uint16_t rgb16_t; // R5G6B5

/** @brief Background color number (4 bit), Forground color number (4 bit) */
typedef uint8_t colorbyte_t; // BG4FG4

/** @brief CGA/VGA 16 Color numbers. */
typedef enum colorn16 {
    C16_BLACK = 0,
    C16_BLUE,
    C16_GREEN,
    C16_CYAN,
    C16_RED,
    C16_MAGENTA,
    C16_BROWN,
    C16_WHITE,
    C16_GREY,
    C16_LT_BLUE,
    C16_LT_GREEN,
    C16_LT_CYAN,
    C16_ORANGE,
    C16_VIOLET,
    C16_YELLOW,
    C16_BR_WHITE
} colorn16_t;

/**
 * @brief Screen line & column position.
 * @ingroup display
 */
typedef struct scr_position {
    unsigned short line;
    unsigned short column;
} scr_position_t;


/**
 * @brief Create 'color-byte number' from forground & background color numbers.
 * @ingroup display
 *
 * @param fg Forground color-16 number
 * @param gb Background color-16 number
 */
colorbyte_t colorbyte(colorn16_t fg, colorn16_t bg);

/**
 * @brief Get forground color number from color-byte number.
 * @ingroup display
 *
 * @param  cb Color-byte value
 */
colorn16_t fg_from_cb(colorbyte_t cb);

/**
 * @brief Get background color number from color-byte number
 * @ingroup display
 *
 * @param cb Color-byte value
 */
colorn16_t bg_from_cb(colorbyte_t cb);

/**
 * @brief Get a RGB-16 (R5G6B5) value from a Color-16 (0-15 Color number)
 * @ingroup display
 *
 * @param cn16 Color-16 number to get a RGB-16 (R5G6B5) value for
 */
rgb16_t rgb16_from_color16(colorn16_t cn16);

/**
 * @brief Get the current cursor position
 * @ingroup display
 *
 * @returns Cursor screen position
 */
scr_position_t cursor_get(void);

/**
 * @brief Position the cursor in the top-left corner of the screen as defined by
 *  the margins.
 * @ingroup display
 */
void cursor_home(void);

/**
 * @brief Set the cursor location.
 * @ingroup display
 *
 * The cursor is used for `print` operations. It advances as characters are printed.
 *
 * @param line 0-based line to move the cursor to.
 * @param col 0-based column to move the cursor to.
 */
void cursor_set(unsigned short line, unsigned short col);

/**
 * @brief Set the cursor location.
 * @ingroup display
 *
 * The cursor is used for `print` operations. It advances as characters are printed.
 *
 * @param pos 0-based `scr_position_t` (line/column) to move the cursor to.
 */
void cursor_set_sp(scr_position_t pos);

/**
 * @brief Clear the text screen
 * @ingroup display
 *
 * Clear the current text content and the screen.
 *
 *  @param paint Set true to paint the screen after the operation. Otherwise, only buffers will be cleared.
*/
void disp_clear(bool paint);

/**
 * @brief Display a character on the text screen
 * @ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 *
 * @param line Line number, with 0 being the top line
 * @param col Column number, with 0 being the leftmost column
 * @param c character to display
 * @param paint True to paint the screen after the operation.
 */
void disp_char(unsigned short int line, unsigned short int col, char c, bool paint);

/**
 * @brief Display a character on the text screen with a given forground and background color.
 * @ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 *
 * @param line Line number, with 0 being the top line
 * @param col Column number, with 0 being the leftmost column
 * @param c character to display
 * @param fg forground color number (0-15)
 * @param bg background color number (0-15)
 * @param paint True to paint the screen after the operation.
 */
void disp_char_color(unsigned short int line, unsigned short int col, char c, uint8_t fg, uint8_t bg, bool paint);

/**
 * @brief Display a character on the text screen with a given forground and background color.
 * @ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 *
 * @param line 0-5 With 0 being the top line
 * @param col 0-13 Starting column
 * @param c character to display
 * @param color forground and background color as a `colorbyte` (HN:0-15 background, LN:0-15 forground)
 * @param paint True to paint the screen after the operation.
 */
void disp_char_colorbyte(unsigned short int line, unsigned short int col, char c, uint8_t color, bool paint);

/**
 * @brief Test the fonts by displaying all of the characters
 * @ingroup display
 *
 * Display all of the font characters a page at a time. Pause between pages and overlap
 * the range of characters some from page to page.
 *
 */
void disp_font_test(void);

/**
 * @brief Initialize the display
 * @ingroup display
 *
 * This must be called before using the display.
 *
 */
void disp_init(void);

/**
 * @brief Paint the actual display screen
 * @ingroup display
 *
 * To improve performance and the look of the display, most changes can be made without
 * updating the physical display. Then, once a batch of changes have been made, this
 * is called to move the screen/image buffer onto the display.
 */
void disp_paint(void);

/**
 * @brief Clear the character line.
 * @ingroup display
 *
 *  @param line The 0-based line to clear.
 * @param paint True to paint the screen after the operation.
*/
void disp_line_clear(unsigned short int line, bool paint);

/**
 * @brief Paint the portion of the screen containing the given character line.
 * @ingroup display
 *
 *  This 'paints' the screen from the display buffer. To paint the buffer
 *  from the line data use `disp_row_refresh`.
 *
 *  @param line The 0-based character line to paint.
*/
void disp_line_paint(unsigned short int line);

/**
 * @brief Scroll 2 or more rows up.
 * @ingroup display
 *
 *  Scroll the character rows up, removing the top line and
 *  clearing the bottom line.
 *
 *  @param row_t The 0-based top line.
 *  @param row_b The 0-based bottom line.
 * @param paint True to paint the screen after the operation.
*/
void disp_rows_scroll_up(unsigned short int row_t, unsigned short int row_b, bool paint);

/**
 * @brief Set the text forground and background colors to be used for placed text.
 * @ingroup display
 *
 * Sets the colors to be used by default when placing text. The colors are from the
 * 16 color (VGA) set.
 *
 * @param fg Color number (0-15) for the forground
 * @param bg Color number (0-15) for the background
*/
void disp_set_text_colors(uint8_t fg, uint8_t bg);

/**
 * @brief Display a string
 * @ingroup display
 *
 * Display a string of ASCII characters (plus some special characters)
 *
 * @param line Line number, with 0 being the top line
 * @param col Column number, with 0 being the leftmost column
 * @param pString Pointer to the first character of a null-terminated string
 * @param invert True to invert the characters
 * @param paint True to paint the screen after the operation
 */
void disp_string(unsigned short int line, unsigned short int col, const char *pString, bool invert, bool paint);

/**
 * @brief Update the display (graphics) buffer from the line data. Optionally paint the screen
 * @ingroup display
 *
 *  @param paint True to paint the screen after the operation.
*/
void disp_update(bool paint);

/**
 * @brief Clear the screen and display the 16 VGA Colors.
 * @ingroup display
 */
void disp_c16_color_chart();

/**
 * @brief Set the margins for cursor based printing.
 * @ingroup display
 *
 * @param top 0-based line of the top margin (top side of text window).
 * @param left 0-based column of the left margin (left side of text window).
 * @param bottom 0-based line of the bottom margin (bottom of text window).
 * @param right 0-based column of the right margin (right side of text window).
 */
void margins_set(unsigned short top, unsigned short left, unsigned short bottom, unsigned short right);

/**
 * @brief Print a character at the current cursor location. Advance the cursor.
 * @ingroup display
 *
 * This will print a single character and advance the cursor. The newline character is not special,
 * and the graphics character for '0x0A' will be printed. To get a new line, use `print_crlf` or
 * `prints` with a single-character string containing a newline.
 *
 * @param c Character to print. If the top bit is set, the character will be inverted.
 * @param paint True to paint the screen after the operation.
 */
void printc(char c, bool paint);

/**
 * @brief Move the cursor to the beginning of the next line. Scroll the display if needed.
 * 
 * @param add_lines Additional lines to advance the cursor.
 * @param paint True to paint the screen after the operation.
 */
void print_crlf(short add_lines, bool paint);

/**
 * @brief Print a string at the current cursor location. Advance the cursor and scroll if needed.
 * @ingroup display
 *
 * Print a string and advance the cursor. A newline within the string will move the cursor
 * to the next line until it hits the bottom margin. When the bottom margin is hit, the lines will be
 * scrolled up to make room for a blank line.
 *
 * @param s Null terminated string. If the top bit of a character is set, the character will be inverted.
 * @param paint True to paint the screen after the operation.
 */
void prints(char* s, bool paint);

/**
 * @brief `printf` that goes to the display using the current cursor position and advancing the cursor.
 * @ingroup display
 *
 * @param paint True to paint the screen after the operation.
 * @param format Format string that follows the standard `printf` formatting.
 * @param ... Variable arguments used to satisfy the format.
 *
 * @return The number of characters printed.
 */

int disp_printf(bool paint, const char* format, ...) __attribute__((format(_printf_, 2, 3)));

#ifdef __cplusplus
}
#endif
#endif // DISPLAY_H

