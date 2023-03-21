/**
 * MuKOB User Interface - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _UI_TERM_H_
#define _UI_TERM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt.h"
#include "term.h"

#define UI_TERM_NAME_VERSION "μKOB v0.1"

// NOTE: Terminal line and column numbers are 1-based.

#define UI_TERM_COLUMNS 80 // VT/ANSI indicate valid values are 80|132, emulators take others
#define UI_TERM_LINES 36 // VT/ANSI indicate: 24, 25, 36, 42, 48, 52, and 72 are valid

// Code display color
#define UI_TERM_CODE_COLOR_FG TERM_CHR_COLOR_GREEN
#define UI_TERM_CODE_COLOR_BG TERM_CHR_COLOR_BLACK

// Command color
#define UI_TERM_CMD_COLOR_FG TERM_CHR_COLOR_BR_CYAN
#define UI_TERM_CMD_COLOR_BG TERM_CHR_COLOR_BLACK

// Top header and gap
#define UI_TERM_HEADER_COLOR_FG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_HEADER_COLOR_BG TERM_CHR_COLOR_BLUE
#define UI_TERM_HEADER_INFO_LINE 1
#define UI_TERM_HEADER_SPEED_LABEL_COL 14
#define UI_TERM_HEADER_SPEED_VALUE_COL 20
#define UI_TERM_HEADER_WIRE_LABEL_COL 5
#define UI_TERM_HEADER_WIRE_VALUE_COL 10

// Current sender line (at the top)
#define UI_TERM_SENDER_COLOR_FG TERM_CHR_COLOR_BLUE
#define UI_TERM_SENDER_COLOR_BG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_SENDER_LINE 2

// Station list
#define UI_TERM_STATION_LIST_START_LINE (UI_TERM_LINES - 1 - 4)
#define UI_TERM_STATION_LIST_END_LINE (UI_TERM_STATION_LIST_START_LINE + 3)
#define UI_TERM_STATION_LIST_LINES (UI_TERM_STATION_LIST_END_LINE - UI_TERM_STATION_LIST_START_LINE)

// Bottom status
#define UI_TERM_STATUS_COLOR_FG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_STATUS_COLOR_BG TERM_CHR_COLOR_BLUE
#define UI_TERM_STATUS_LINE (UI_TERM_LINES)
#define UI_TERM_STATUS_LOGO_COL (UI_TERM_COLUMNS - 3)
#define UI_TERM_STATUS_TIME_COL ((UI_TERM_COLUMNS / 2) - 3)

// Scroll margins
#define UI_TERM_SCROLL_START_LINE 3
#define UI_TERM_SCROLL_END_LINE (UI_TERM_STATION_LIST_START_LINE - 1)

// Command line (last line of scroll area)
#define UI_TERM_CMDLINE (UI_TERM_SCROLL_END_LINE)

// Labels
#define UI_TERM_WIRE_LABEL "Wire:"
#define UI_TERM_SPEED_LABEL "Speed:"
#define AES_LOGO "AES"

typedef struct _TERM_COLOR_PAIR_ {
    term_color_t fg;
    term_color_t bg;
} term_color_pair_t;

/**
 * @brief Callback function that gets the line once it has been received/assembled.
 * @ingroup ui
 *
 * Because the system is message based for cooperative multi-tasking. It isn't possible
 * to block when trying to read a line from the terminal. To support getting an input
 * line from the user (echoing characters and handling backspace) as easily as possible
 * for things like the command processor and dialogs. The work is handled by the this
 * `term` module and a function is called from the main message loop once the line is
 * ready.
 *
 * @param line Pointer to the line buffer. The line buffer allows modification to support
 * parsing operations without needing to be copied. However, the buffer is statically
 * allocated and used for any call to `term_getline`. Therefore, if `term_getline` is
 * to be called while the current line contents are being used, the caller will need to
 * copy the line contents needed before making the call.
 *
 */
typedef void (*ui_term_getline_callback_fn)(char* line);

/**
 * @brief Function prototype for handling an terminal input character available notification.
 * @ingroup ui
 *
 * This is used to handle the input on the UI loop (core) rather than in an interrupt handler.
 */
typedef void (*ui_term_input_available_handler)(void);

/**
 * @brief Message handler for MSG_INIT_TERMINAL
 * @ingroup ui
 *
 * Init/re-init the terminal. This is typically received by a user requesting
 * that the terminal be re-initialized/refreshed. For example if they connect
 * a terminal after MuKOB is already up and running.
 *
 * @param msg Nothing important in the message.
 */
extern void _ui_term_handle_init_terminal(cmt_msg_t* msg);

/**
 * @brief `MSG_INPUT_CHAR_READY` message handler.
 * @ingroup ui
 *
 * @param msg Nothing important in the message.
 */
extern void _ui_term_handle_input_char_ready(cmt_msg_t* msg);

/**
 * @brief Build (or rebuild) the UI on the terminal.
 * @ingroup ui
 */
extern void ui_term_build(void);

/**
 * @brief Get the current terminal color pair.
 * @ingroup ui
 */
extern term_color_pair_t ui_term_color_get();

/**
 * @brief Set the terminal the currently set text colors.
 * @ingroup ui
 */
extern void ui_term_color_refresh();

/**
 * @brief Set and save text forground and background colors.
 * @ingroup ui
 *
 * This should be used to set a screen color that can restored when needed.
 *
 * @param fg The color number for the foreground
 * @param bg The color number for the background
 */
extern void ui_term_color_set(term_color_t fg, term_color_t bg);

/**
 * @brief Refresh the speed value display in the header.
 * @ingroup ui
 */
extern void ui_term_display_speed();

/**
 * @brief Refresh the wire number display in the header.
 * @ingroup ui
 */
extern void ui_term_display_wire();

/**
 * @brief Get a line of user input. Returns immediately. Calls back when line is ready.
 * @ingroup term
 * @see term_getline_callback_fn for details on use.
 *
 * @param getline_cb The callback function to call when a line is ready.
 */
extern void ui_term_getline(ui_term_getline_callback_fn getline_cb);

/**
 * @brief Register a function to handle terminal input available.
 *
 * @param handler_fn
 */
extern void ui_term_register_input_available_handler(ui_term_input_available_handler handler_fn);

/**
 * @brief Set the color to the code display color.
 */
extern void ui_term_use_code_color();

/**
 * @brief Set the color to the command display color.
 */
extern void ui_term_use_cmd_color();

/**
 * @brief Update the sender station ID in the top of the terminal.
 * @ingroup ui
 *
 * @param id The station ID of the sender. A NULL will clear the sender line.
 */
extern void ui_term_update_sender(const char* id);

/**
 * @brief Update the speed value.
 * @ingroup ui
 *
 * @param speed The speed in WPM
 */
extern void ui_term_update_speed(uint16_t speed);

/**
 * @brief Update the status bar.
 * @ingroup ui
 */
extern void ui_term_update_status();

/**
 * @brief Update/refresh the wire number.
 * @ingroup ui
 *
 * @param wire The wire number.
 */
extern void ui_term_update_wire(uint16_t wire);

#ifdef __cplusplus
    }
#endif
#endif // _UI_TERM_H_