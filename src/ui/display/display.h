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
#ifndef _DISPLAY_H_
#define _DISPLAY_H_
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

/** @brief Use in display method calls to cause the screen to be printed */
typedef enum Paint_Control_ {
    No_Paint = 0,
    Paint,
} paint_control_t;

/**
 * @brief Screen line & column position.
 * @ingroup display
 */
typedef struct _scr_position_ {
    uint16_t line;
    uint16_t column;
} scr_position_t;

/**
 * @brief Text foreground and background color pair.
 */
typedef struct _text_color_pair_ {
    colorn16_t fg;
    colorn16_t bg;
} text_color_pair_t;

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
 * @returns Cursor screen position within the scroll area.
 */
scr_position_t cursor_get(void);

/**
 * @brief Position the cursor in the top-left corner of the screen as defined by
 *  the scroll area.
 * @ingroup display
 */
void cursor_home(void);

/**
 * @brief Show or hide the cursor.
 * @ingroup display
 *
 * The cursor is used for `print...` operations whether or not the cursor is
 * shown.
 *
 * @param show True to show the cursor, false to hide it.
 */
void cursor_show(bool show);

/**
 * @brief Set the cursor location.
 * @ingroup display
 *
 * The cursor is used within the scroll area for `print` operations.
 * It advances as characters are printed.
 *
 * @param line 0-based line to move the cursor to within the scroll area.
 * @param col 0-based column to move the cursor to within the scroll area.
 */
void cursor_set(uint16_t line, uint16_t col);

/**
 * @brief Set the cursor location.
 * @ingroup display
 *
 * The cursor is used within the scroll area for `print` operations.
 * It advances as characters are printed.
 *
 * @param pos 0-based `scr_position_t` (line/column) to move the cursor to within the scroll area.
 */
void cursor_set_sp(scr_position_t pos);

/**
 * @brief Clear the screen and display the 16 VGA Colors.
 * @ingroup display
 */
void disp_c16_color_chart();

/**
 * @brief Clear the entire screen
 * @ingroup display
 *
 * @param paint Controls painting of the screen after the operation.
*/
void disp_clear(paint_control_t paint);

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
 * @param paint Controls painting of the screen after the operation.
 */
void disp_char(uint16_t line, uint16_t col, char c, paint_control_t paint);

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
 * @param paint Controls painting of the screen after the operation.
 */
void disp_char_color(uint16_t line, uint16_t col, char c, colorn16_t fg, colorn16_t bg, paint_control_t paint);

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
 * @param paint Controls painting of the screen after the operation.
 */
void disp_char_colorbyte(uint16_t line, uint16_t col, char c, colorbyte_t color, paint_control_t paint);

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
 * @brief Get the current text colors.
 * 
 * @return text_color_pair_t The current foreground and background colors.
 */
void disp_get_text_colors(text_color_pair_t* cp);

/**
 * @brief Display info - number of text columns
 *
 * Columns are numbered 0 through number of columns minus 1.
 *
 * @return uint16_t The number of text columns
 */
uint16_t disp_info_columns();

/**
 * @brief Display info - number of text lines
 *
 * Lines are numbered 0 through number of lines minus 1.
 *
 * @return uint16_t The number of text lines
 */
uint16_t disp_info_lines();

/**
 * @brief Display info - number of fixed text lines at the top of the display
 *
 * @return uint16_t The number of fixed lines
 */
uint16_t disp_info_fixed_top_lines();

/**
 * @brief Display info - number of fixed text lines a the bottom of the display
 *
 * @return uint16_t The number of fixed lines
 */
uint16_t disp_info_fixed_bottom_lines();

/**
 * @brief Display info - number of lines that scroll
 *
 * The scroll lines are the lines between the top fixed area and the bottom fixed
 * area. There can be as few as 0, and as many as the full screen, scroll lines.
 * The scroll lines contain the cursor (used with the `print...` functions), which
 * advances as text is printed.
 *
 * @return uint16_t The number of text lines that scroll
 */
uint16_t disp_info_scroll_lines();

/**
 * @brief Initialize the display
 * @ingroup display
 *
 * This must be called before using the display.
 *
 */
void disp_init(void);

/**
 * @brief Clear the character line.
 * @ingroup display
 *
 * @param line The 0-based line to clear.
 * @param paint Controls painting of the screen after the operation.
*/
void disp_line_clear(uint16_t line, paint_control_t paint);

/**
 * @brief Paint the portion of the screen containing the given character line.
 * @ingroup display
 *
 *  This 'paints' the screen from the display buffer. To paint the buffer
 *  from the line data use `disp_row_refresh`.
 *
 * @param line The 0-based character line to paint.
*/
void disp_line_paint(uint16_t line);

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
 * @brief Set the text forground and background colors to be used for placed text.
 * @ingroup display
 *
 * Sets the colors to be used by default when placing text. The colors are from the
 * 16 color number (VGA) set.
 *
 * @param fg Color number (0-15) for the forground
 * @param bg Color number (0-15) for the background
*/
void disp_set_text_colors(colorn16_t fg, colorn16_t bg);

/**
 * @brief Set the text forground and background colors to be used for placed text.
 * @ingroup display
 *
 * Sets the colors to be used by default when placing text. The colors are from the
 * 16 color number (VGA) set.
 *
 * @param cp Pointer to a text color pair to use to set the colors.
 */
void disp_set_text_colors_cp(text_color_pair_t* cp);

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
void disp_string(uint16_t line, uint16_t col, const char *pString, bool invert, paint_control_t paint);

/**
 * @brief Display a string with a given forground and background color
 * @ingroup display
 *
 * Display a string of ASCII characters (plus some special characters)
 *
 * @param line Line number, with 0 being the top line
 * @param col Column number, with 0 being the leftmost column
 * @param pString Pointer to the first character of a null-terminated string
 * @param fg Forground color number (0-15)
 * @param bg Background color number (0-15)
 * @param paint True to paint the screen after the operation
 */
void disp_string_color(uint16_t line, uint16_t col, const char* pString, colorn16_t fg, colorn16_t bg, paint_control_t paint);

/**
 * @brief Update the display (graphics) buffer from the text line data. Optionally paint the screen
 * @ingroup display
 *
 * @param paint True to paint the screen after the operation.
*/
void disp_update(paint_control_t paint);

/**
 * @brief Set the margins for cursor based printing.
 * @ingroup display
 *
 * @param top 0-based line of the top margin (top side of text window).
 * @param left 0-based column of the left margin (left side of text window).
 * @param bottom 0-based line of the bottom margin (bottom of text window).
 * @param right 0-based column of the right margin (right side of text window).
 */
//void margins_set(unsigned short top, unsigned short left, unsigned short bottom, unsigned short right);

/**
 * @brief Print a character at the current cursor location. Advance the cursor.
 * @ingroup display
 *
 * This will print a single character and advance the cursor. The newline character is not special,
 * and the graphics character for '0x0A' will be printed. To get a new line, use `print_crlf` or
 * `prints` with a single-character string containing a newline.
 *
 * @param c Character to print. If the top bit is set, the character will be inverted.
 * @param paint Controls painting of the screen after the operation.
 */
void printc(char c, paint_control_t paint);

/**
 * @brief Move the cursor to the beginning of the next line. Scroll the display if needed.
 *
 * @param add_lines Additional lines to advance the cursor.
 * @param paint Controls painting of the screen after the operation.
 */
void print_crlf(int16_t add_lines, paint_control_t paint);

/**
 * @brief `printf` that goes to the display using the current cursor position and advancing the cursor.
 * @ingroup display
 *
 * @param paint Controls painting of the screen after the operation.
 * @param format Format string that follows the standard `printf` formatting.
 * @param ... Variable arguments used to satisfy the format.
 *
 * @return The number of characters printed.
 */
int printf_disp(paint_control_t paint, const char* format, ...) __attribute__((format(_printf_, 2, 3)));

/**
 * @brief Print a string at the current cursor location. Advance the cursor and scroll if needed.
 * @ingroup display
 *
 * Print a string and advance the cursor. A newline within the string will move the cursor
 * to the next line until it hits the bottom margin. When the bottom margin is hit, the lines will be
 * scrolled up to make room for a blank line.
 *
 * @param s Null terminated string. If the top bit of a character is set, the character will be inverted.
 * @param paint Controls painting of the screen after the operation.
 */
void prints(char* s, paint_control_t paint);

/**
 * @brief Closes the active screen and restores the previous screen.
 *
 * New screens are created with `screen_new`. When done with a screen
 * (say for a dialog), this is called to close the screen and go back to
 * the prior screen.
 *
 * @see screen_new()
 */
void screen_close();

/**
 * @brief Create a new (sub) screen to work with.
 *
 * This creates a new screen context and makes it the current context. The previous screen is saved
 * until this one is closed. The screens are kept on a stack. When done with the screen, use
 * `screen_close` to close it and return to the previous screen.
 *
 * @see screen_close()
 *
 * @return true If the new screen was created and made current.
 * @return false If the new screen couldn't be created or made current.
 */
bool screen_new();

/**
 * @brief Clear the scroll area of the screen.
 * @ingroup display
 *
 * @param paint Set true to paint the screen after the operation. Otherwise, only buffers will be cleared.
 */
void scroll_area_clear(paint_control_t paint);

/**
 * @brief Define the scroll area of the screen by defining the top and bottom fixed areas.
 *
 * The scroll area is where the (primary) cursor resides for print operations. This also positions
 * the cursor at the home position within the scroll area.
 *
 * Be aware that if the entire screen is 'fixed' the functions that use the cursor (the `print...`
 * functions and some others) become 'no-op'.
 *
 * @param top_fixed_size
 * @param bottom_fixed_size
 */
void scroll_area_define(uint16_t top_fixed_size, uint16_t bottom_fixed_size);

#ifdef __cplusplus
}
#endif
#endif // _DISPLAY_H_

