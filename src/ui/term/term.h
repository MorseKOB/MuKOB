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

// Define to debug terminal control sequences
//#define _TERM_CONTROL_DEBUG_

#ifndef _TERM_CONTROL_DEBUG_
#define ESC '\e'
#define CSI "\e["
#define DCS "\eP"
#define SS3 "\eO"
#define IND "\eD"
#else
// Temp debug
#define ESC 'E'
#define CSI "ESC["
#define DCS "ESCP"
#define SS3 "ESCO"
#define IND "ESCD"
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
 * @brief VT Terminal type specifiers used for the 'CSI 6 x " p' (DECSCL) control.
 *
 */
typedef enum _VT_TERM_TYPE_SPECIFIER_ {
    VT_102_TYPE_SPEC                = 1,
    VT_220_TYPE_SPEC                = 2,
    VT_320_TYPE_SPEC                = 3,
    VT_510_TYPE_SPEC                = 4,
} vt_term_type_spec_t;

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
 * @brief Save the cursor position and attributes.
 *
 * @see term_cursor_restore
 */
extern void term_cursor_save(void);

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
 * @brief Send a reset (ESC c) to the terminal.
 *
 * This will reset (most of) the terminal attributes to their power on / reset values.
 * The PuTTY docs indicate that it will not reset the terminal emulation/type.
 *
 */
extern void term_reset(void);

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
extern void term_set_type_vt(vt_term_type_spec_t type);

#ifdef __cplusplus
}
#endif
#endif // _TERM_H_