/**
 * @brief Terminal functionality.
 * @ingroup display
 *
 * This provides a very (very, very) simple NCURSES-like functionality.
 * It is hard-coded to expect an ANSI capable terminal. It is tested primarily
 * with Putty.
 *
 * @see https://www.putty.org/ (Windows)
 * @see https://www.puttygen.com/download-putty#Download_PuTTY_for_Mac_and_Installation_Guide (Mac)
 * @see https://www.puttygen.com/download-putty#Download_PuTTY_on_Linux_and_Installation_Guide (Linux)
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _TERM_H_
#define _TERM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display.h"

// Define to debug terminal control sequences
//#define _TERM_CONTROL_DEBUG_

#ifndef _TERM_CONTROL_DEBUG_
#define BS      '\010'  // Backspace
#define CSI     "\e["   // Control Sequence Introducer
#define DCS     "\eP"   // Device Control String
#define ENQ     '\005'  // ^E (ENQ)
#define ESC     '\e'    // Escape
#define IND     "\eD"   // Index
#define NEL     "\eE"   // Next Line
#define OSC     "\e]"   // Operating System Command
#define RI      "\eM"   // Reverse Index (UP 1)
#define ST      "\e\\"  // String Terminator
#define SS3     "\eO"   // Single Shift 3
#else
// Temp debug
#define BS      'ß'
#define CSI     "ESC["
#define DCS     "ESCP"
#define ENQ     'Ê'
#define ESC     'Ë'
#define IND     "ESCD"
#define NEL     "ÑL"
#define OSC     "ESC]"
#define RI      "Û"
#define ST      "ESC\\"
#define SS3     "ESCO"
#endif

/**
 * @brief Terminal text colors
 */
typedef enum _TERM_CHR_COLOR_NUMS_ {
    TERM_CHR_COLOR_BLACK            =  0,
    TERM_CHR_COLOR_RED              =  1,
    TERM_CHR_COLOR_GREEN            =  2,
    TERM_CHR_COLOR_YELLOW           =  3,
    TERM_CHR_COLOR_BLUE             =  4,
    TERM_CHR_COLOR_MAGENTA          =  5,
    TERM_CHR_COLOR_CYAN             =  6,
    TERM_CHR_COLOR_WHITE            =  7,
    TERM_CHR_COLOR_GRAY             =  8,
    TERM_CHR_COLOR_BR_RED           =  9,
    TERM_CHR_COLOR_BR_GREEN         =  10,
    TERM_CHR_COLOR_BR_YELLOW        =  11,
    TERM_CHR_COLOR_BR_BLUE          =  12,
    TERM_CHR_COLOR_BR_MAGENTA       =  13,
    TERM_CHR_COLOR_BR_CYAN          =  14,
    TERM_CHR_COLOR_BR_WHITE         =  15,
} term_color_t;

/**
 * @brief Terminal Origin Mode (DECOM)
 *
 * Upper Left allows the cursor to move to any point in the screen.
 * Margins keeps the cursor within the margins.
 *
 */
typedef enum _TERM_ORIGIN_MODE_ {
    TERM_OM_UPPER_LEFT              = 0,
    TERM_OM_IN_MARGINS              = 1,
} term_om_t;

/**
 * @brief VT Terminal type specifiers used for the 'CSI 6 <n> " p' (DECSCL) control.
 *
 */
typedef enum _VT_TERM_TYPE_SPECIFIER_ {
    VT_102_TYPE_SPEC                = 1,
    VT_220_TYPE_SPEC                = 2,
    VT_320_TYPE_SPEC                = 3,
    VT_510_TYPE_SPEC                = 4,
} vt_term_type_spec_t;

/**
 * @brief Specifier for terminal ID used for the 'CSI <n> , q' (DECTID) report.
 *
 */
typedef enum _VT_TERM_ID_SPECIFIER_ {
    VT_102_ID_SPEC = 2,
    VT_220_ID_SPEC = 5,
    VT_320_ID_SPEC = 7,
    VT_420_ID_SPEC = 9,
    VT_510_ID_SPEC = 10,
} vt_term_id_spec_t;

/**
 * @brief Function prototype for UDP Bind response handler.
 * @ingroup wire
 *
 * @param status The statuc from the operation.
 * @param udp_pcb The udp_pcb that was bound, or NULL if an error occurred.
 */
typedef void (*term_notify_on_input_fn)(void);

/**
 * @brief Clear the full screen
 *
 */
extern void term_clear(void);

/**
 * @brief Set the text character forground color using a color value.
 *
 * @param colorn A color number (0-16)
 */
extern void term_color_fg(term_color_t colorn);

/**
 * @brief Set the text character background color using a color value.
 *
 * @param colorn A color number (0-16)
 */
extern void term_color_bg(term_color_t colorn);

/**
 * @brief Set the text to the default colors.
 *
 */
extern void term_color_default(void);

/**
 * @brief Move the cursor down `n` lines.
 *
 * @param n Number of lines to move.
 */
extern void term_cursor_down(uint16_t n);

/**
 * @brief Move the cursor down 1 line.
 *
 */
extern void term_cursor_down_1(void);

/**
 * @brief Move the cursor left `n` columns.
 *
 * @param n Number of columns to move.
 */
extern void term_cursor_left(uint16_t n);

/**
 * @brief Move the cursor left 1 column.
 *
 */
extern void term_cursor_left_1(void);

/**
 * @brief Move the cursor to a given line and column.
 *
 * @param line Terminal line (top is 1)
 * @param column Terminal column (left is 1)
 */
extern void term_cursor_moveto(uint16_t line, uint16_t column);

/**
 * @brief Turn the cursor on/off (visible/hidden)
 *
 * @param on True shows the cursor. False hides the cursor.
 */
extern void term_cursor_on(bool on);

/**
 * @brief Restore the cursor position and attributes.
 *
 * Restore the cursor from that previously saved.
 *
 * @see term_cursor_save
 */
extern void term_cursor_restore(void);

/**
 * @brief Move the cursor right `n` columns.
 *
 * @param n Number of columns to move.
 */
extern void term_cursor_right(uint16_t n);

/**
 * @brief Move the cursor right 1 column.
 *
 */
extern void term_cursor_right_1(void);

/**
 * @brief Save the cursor position and attributes.
 *
 * @see term_cursor_restore
 */
extern void term_cursor_save(void);

/**
 * @brief Move the cursor up `n` lines.
 *
 * @param n Number of lines to move.
 */
extern void term_cursor_up(uint16_t n);

/**
 * @brief Move the cursor down 1 line.
 *
 */
extern void term_cursor_up_1(void);

/**
 * @brief Erase from the cursor to the beginning of the line.
 *
 */
extern void term_erase_bol(void);

/**
 * @brief Erase from the cursor to the end of the line.
 *
 */
extern void term_erase_eol(void);

/**
 * @brief Erase the line the cursor is on.
 *
 */
extern void term_erase_line(void);

/**
 * @brief Get a character with a timeout in micro-seconds.
 *
 * Once the terminal has been initialized, this should be used rather than
 * getchar/getchar_timeout_us as the terminal support installs a handler
 * and implements an input buffer.
 *
 * @see getchar()
 * @see getchar_timeout_us(us)
 * @see term_input_available()
 *
 * @return int The input character or -1 if no character is available.
 */
extern int term_getc(void);

/**
 * @brief Get the current cursor position.
 *
 * Sends a CPR (CSI 6 n) to the terminal and reads and processes the response. It waits
 * a maximum of 150ms for a response. If no response is received a position of -1,-1
 * is returned.
 *
 * @return scr_position_t
 */
extern scr_position_t term_get_cursor_position(void);

/**
 * @brief Get the terminal screen info.
 *
 * This sends a CPR (CSI 6 n) to the terminal and reads the tesponse. It waits a maximum of
 * 250ms for a respoinse.
 *
 * @param buf A character buffer to store the DA string into.
 * @param maxlen The maximum number of characters to get.
 *
 * @return int The number of characters actually read.
 */
extern int term_get_screen_info(char* buf, int maxlen);

/**
 * @brief
 *
 * @param id_spec The term type to get the ID info for
 * @param buf  A character buffer to store the ID string into.
 * @param maxlen The maximum number of characters to get.

 * @return int The number of characters actually read.
 */
extern int term_get_id_info(vt_term_id_spec_t id_spec, char* buf, int maxlen);

/**
 * @brief Get the term name (response to ^E/ENQ).
 *
 * @param buf  A character buffer to store the name string into.
 * @param maxlen The maximum number of characters to get.

 * @return int The number of characters actually read.
 */
extern int term_get_name(char* buf, int maxlen);

/**
 * @brief Get the terminal device attributes (1).
 *
 * This sends a DA1 (CSI 0 c) to the terminal and reads the response. It waits a maximum of
 * 250ms for a respoinse.
 *
 * @param buf A character buffer to store the DA string into.
 * @param maxlen The maximum number of characters to get.
 *
 * @return int The number of characters actually read.
 */
extern int term_get_type_info(char* buf, int maxlen);

/**
 * @brief Initialize the Term library and send initial configuration to the terminal.
 *
 * This initializes the terminal type and screen size and sets up an input handler
 * from the standard input device.
 */
extern void term_init(void);

/**
 * @brief Return status of available input.
 *
 * @return true If there is input data available (in the input buffer).
 * @return false If there isn't currently input data available.
 */
extern bool term_input_available(void);

/**
 * @brief Clears the input buffer.
 *
 */
extern void term_input_buf_clear(void);

/**
 * @brief Indicates if input data was lost (the buffer was full when input was received).
 *
 * Once set, this status will remain true until this function is called, or the input buffer is cleared.
 *
 * @see term_input_but_clear()
 *
 * @return true Input was lost since the last time this was called.
 * @return false No input has been lost since the last time this was called.
 */
extern bool term_input_overflow(void);

/**
 * @brief Register a function to be called when input data becomes available.
 *
 * The function will be called when input data becomes available, including if
 * data is currently available. This is a one-time operation. Once it is called,
 * the function is de-registered.
 *
 * @param fn A `void funcion(void)` function to be called, or `NULL` to remove a registered function.
 */
extern void term_notify_on_input(term_notify_on_input_fn notify_fn);

/**
 * @brief Terminal ID returned upon power-up.
 *
 * @return const char* The terminal id (null terminated).
 */
extern const char* term_pu_id(void);

/**
 * @brief Terminal name returned upon power-up.
 *
 * @return const char* The terminal name (null terminated).
 */
extern const char* term_pu_name(void);

/**
 * @brief Send a reset (ESC c) to the terminal.
 *
 * This will reset (most of) the terminal attributes to their power on / reset values.
 * The PuTTY docs indicate that it will not reset the terminal emulation/type.
 *
 */
extern void term_reset(void);

/**
 * @brief Set the terminal top margin and bottom margin.
 *
 * This defines the scroll size if the origin mode is set to TERM_OM_IN_MARGINS. The bottom margin
 * should be less than or equal to the virtical screen size.
 *
 * @param top_line Top margin (lines less than this will be fixed at the top)
 * @param bottom_line Bottom margin (lines grater than this will be fixed at the bottom)
 */
extern void term_set_margin_top_bottom(uint16_t top_line, uint16_t bottom_line);

/**
 * @brief Sets the Origin Mode (DECOM).
 *
 * @param mode Origin Mode.
 */
extern void term_set_origin_mode(term_om_t mode);

/**
 * @brief Set the terminal size in lines x columns
 *
 * The Dec manual says:
 * Note 1. The page size can be 24, 25, 36, 42, 48, 52, and 72 lines with 80 or 132 columns.
 * (PuTTY accepts any value)
 *
 * @param lines The number of lines
 * @param columns The number of columns
 */
extern void term_set_size(uint16_t lines, uint16_t columns);

/**
 * @brief Set the terminal compatibility/conformance (operating) level to a specific VT type.
 *
 */
extern void term_set_type(vt_term_type_spec_t type, vt_term_id_spec_t id_type);

/**
 * @brief Attempt to set the title bar on the terminal.
 *
 * @param title Null terminated string to attempt to set.
 */
extern void term_set_title(const char *title);

#ifdef __cplusplus
}
#endif
#endif // _TERM_H_