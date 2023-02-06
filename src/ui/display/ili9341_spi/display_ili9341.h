/**
 * Copyright 2023 AESilky
 * 
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef DISPLAY_H
#define DISPLAY_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include "ili9341_spi.h"

// Sizes when using a 10x16 pixel fixed-width font
#define DISP_CHAR_COLS 32
#define DISP_CHAR_ROWS 15

/*! \brief VGA-16 Color numbers. */
typedef enum color16 {
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
} color16_t;

/*! \brief Screen line & column position. */
typedef struct scr_position {
    unsigned short line;
    unsigned short column;
} scr_position_t;


/*! \brief Create 'color-byte number' from forground & background color numbers */
uint8_t colorbyte(uint8_t fg, uint8_t bg);

/*! \brief Get forground color number from color-byte number */
uint8_t fg_from_cb(uint8_t cb);

/*! \brief Get background color number from color-byte number */
uint8_t bg_from_cb(uint8_t cb);

/*! \brief Create a rgb_t (Red/Green/Blue) from a COLOR-24 (RRGGBB) value */
rgb_t rgb_from_color24(uint32_t color24);

/*! \brief Text character data for the full text screen */
extern uint8_t full_screen_text[DISP_CHAR_ROWS * DISP_CHAR_COLS];
/*! \brief Character color data for the full text screen */
extern uint8_t full_screen_color[DISP_CHAR_ROWS * DISP_CHAR_COLS];

/*! \brief Bit to OR in to invert a character (display black char on white background) */
#define DISP_CHAR_INVERT_BIT 0x80
/*! \brief Mask to AND with a character to remove intert (display white char on black background) */
#define DISP_CHAR_NORMAL_MASK 0x7F

/*! \brief Get the current cursor position 
 *  \ingroup display
 *
 * \returns Cursor screen position
 */
scr_position_t cursor_get(void);

/*! \brief Position the cursor in the top-left corner of the screen as defined by 
 *  the margins.
 *  \ingroup display
 */
void cursor_home(void);

/*! \brief Set the cursor location.
 *  \ingroup display
 *
 * The cursor is used for `print` operations. It advances as characters are printed.
 * 
 * \param line 0-based line to move the cursor to.
 * \param col 0-based column to move the cursor to.
 */
void cursor_set(unsigned short line, unsigned short col);

/*! \brief Set the cursor location.
 *  \ingroup display
 *
 * The cursor is used for `print` operations. It advances as characters are printed.
 * 
 * \param pos 0-based `scr_position_t` (line/column) to move the cursor to.
 */
void cursor_set_sp(scr_position_t pos);

/*! \brief Clear the text screen
 *  \ingroup display
 * 
 * Clear the current text content and the screen.
 * 
 *  \param paint Set true to paint the actual display. Otherwise, only buffers will be cleared.
*/
void disp_clear(bool paint);

/*! \brief Display a character on the text screen
 *  \ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 * 
 * \param row 0-5 With 0 being the top line
 * \param col 0-13 Starting column
 * \param c character to display
 * \param paint Set true to paint the actual display. Otherwise, only buffers will be updated.
 */
void disp_char(unsigned short int row, unsigned short int col, char c, bool paint);

/*! \brief Display a character on the text screen with a given forground and background color.
 *  \ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 * 
 * \param row 0-5 With 0 being the top line
 * \param col 0-13 Starting column
 * \param c character to display
 * \param fg forground color number (0-15)
 * \param bg background color number (0-15)
 * \param paint Set true to paint the actual display. Otherwise, only buffers will be updated.
 */
void disp_char_color(unsigned short int row, unsigned short int col, char c, uint8_t fg, uint8_t bg, bool paint);

/*! \brief Display a character on the text screen with a given forground and background color.
 *  \ingroup display
 *
 * Display an ASCII character (plus some special characters).
 * If the top bit is set (c>127) the character is inverse (black on white background).
 * 
 * \param row 0-5 With 0 being the top line
 * \param col 0-13 Starting column
 * \param c character to display
 * \param color forground and background color as a `colorbyte` (HN:0-15 background, LN:0-15 forground)
 * \param paint Set true to paint the actual display. Otherwise, only buffers will be updated.
 */
void disp_char_colorbyte(unsigned short int row, unsigned short int col, char c, uint8_t color, bool paint);

/*! \brief Test the fonts by displaying all of the characters
 *  \ingroup display
 *
 * Display all of the font characters a page at a time. Pause between pages and overlap 
 * the range of characters some from page to page.
 * 
 */
void disp_font_test(void);

/*! \brief Initialize the display
 *  \ingroup display
 * 
 * This must be called before using the display.
 *
 */
void disp_init(void);

/*! \brief Paint the actual display screen
 *  \ingroup display
 *
 * To improve performance and the look of the display, most changes can be made without 
 * updating the physical display. Then, once a batch of changes have been made, this 
 * is called to move the screen/image buffer onto the display.
 */
void disp_paint(void);

/*! \brief Clear the character row.
 *  \ingroup display
 *
 *  \param row The 0-based row to clear.
 *  \param paint True to also paint the screen.
*/
void disp_row_clear(unsigned short int row, bool paint);

/*! \brief Paint the portion of the screen containing the given character row.
 *  \ingroup display
 *
 *  This 'paints' the screen from the display buffer. To paint the buffer 
 *  from the row data use `disp_row_refresh`.
 * 
 *  \param row The 0-based character row to paint.
*/
void disp_row_paint(unsigned short int row);

/*! \brief Scroll 2 or more rows up.
 *  \ingroup display
 *
 *  Scroll the character rows up, removing the top row and 
 *  clearing the bottom row.
 * 
 *  \param row_t The 0-based top row.
 *  \param row_b The 0-based bottom row.
 *  \param paint True to paint the screen after the operation.
*/
void disp_rows_scroll_up(unsigned short int row_t, unsigned short int row_b, bool paint);

/*! \brief Set the text forground and background colors to be used for placed text. 
 *  \ingroup display
 * 
 * Sets the colors to be used by default when placing text. The colors are from the 
 * 16 color (VGA) set.
 * 
 * \param fg Color number (0-15) for the forground
 * \param bg Color number (0-15) for the background
*/
void disp_set_text_colors(uint8_t fg, uint8_t bg);

/*! \brief Display a string
 *  \ingroup display
 *
 * Display a string of ASCII characters (plus some special characters)
 * 
 * \param row 1-6 With 1 being the top (status) line
 * \param col 1-14 Starting column
 * \param pString Pointer to the first character of a null-terminated string
 * \param invert True to invert the characters
 * \param paint True to paint the screen after the operation
 */
void disp_string(unsigned short int row, unsigned short int col, const char *pString, bool invert, bool paint);

/*! \brief Update the display (graphics) buffer from the row data. Optionally paint the screen 
 *  \ingroup display
 * 
 *  \param paint True to paint the screen after the operation.
*/
void disp_update(bool paint);

/*! \brief Set the margins for cursor based printing. 
 *  \ingroup display
 *
 * \param top 0-based row of the top margin (top side of text window).
 * \param left 0-based column of the left margin (left side of text window).
 * \param bottom 0-based row of the bottom margin (bottom of text window).
 * \param right 0-based column of the right margin (right side of text window).
 */
void margins_set(unsigned short top, unsigned short left, unsigned short bottom, unsigned short right);

/*! \brief Print a character at the current cursor location. Advance the cursor. 
 * \ingroup display
 *
 * This will print a single character and advance the cursor. The newline character is not special, 
 * and will be printed. To get a new line, use `prints` with a single-character string containing 
 * a newline.
 * 
 * \param c Character to print. If the top bit is set, the character will be inverted. 
 * \param paint True to paint the screen after the operation.
 */
void printc(char c, bool paint);

/*! \brief Print a string at the current cursor location. Advance the cursor and scroll if needed. 
 *  \ingroup display
 *
 * Print a string and advance the cursor. A newline within the string will move the cursor 
 * to the next line until it hits the bottom margin. When the bottom margin is hit, the lines will be 
 * scrolled up to make room for a blank line.
 * 
 * \param s Null terminated string. If the top bit of a character is set, the character will be inverted. 
 * \param paint True to paint the screen after the operation.
 */
void prints(char* s, bool paint);

#ifdef __cplusplus
}
#endif
#endif // DISPLAY_H

