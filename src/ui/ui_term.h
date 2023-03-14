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

#define UI_TERM_NAME_VERSION "μKOB v0.1"

// NOTE: Terminal line and column numbers are 1-based.

#define UI_TERM_COLUMNS 80 // VT/ANSI indicate valid values are 80|132, emulators take others
#define UI_TERM_LINES 36 // VT/ANSI indicate: 24, 25, 36, 42, 48, 52, and 72 are valid
#define UI_TERM_SCROLL_START_LINE 3
#define UI_TERM_SCROLL_END_LINE (UI_TERM_LINES - 5)

// Top header and gap
#define UI_TERM_HEADER_COLOR_FG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_HEADER_COLOR_BG TERM_CHR_COLOR_BLUE
#define UI_TERM_HEADER_INFO_LINE 1
#define UI_TERM_HEADER_WIRE_LABEL_COL 5
#define UI_TERM_HEADER_SPEED_LABEL_COL 14

// Current sender line (at the top)
#define UI_TERM_SENDER_COLOR_FG TERM_CHR_COLOR_BLUE
#define UI_TERM_SENDER_COLOR_BG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_SENDER_LINE 2

// Bottom status
#define UI_TERM_STATUS_COLOR_FG TERM_CHR_COLOR_BR_YELLOW
#define UI_TERM_STATUS_COLOR_BG TERM_CHR_COLOR_BLUE
#define UI_TERM_STATUS_LINE (UI_TERM_LINES)
#define UI_TERM_STATUS_LOGO_COL (UI_TERM_COLUMNS - 3)
#define UI_TERM_STATUS_TIME_COL ((UI_TERM_COLUMNS / 2) - 3)

// Command line (last line of scroll area)
#define UI_TERM_CMDLINE (UI_TERM_STATUS_LINE - 2)

// Labels
#define UI_TERM_WIRE_LABEL "Wire:"
#define UI_TERM_SPEED_LABEL "Speed:"
#define AES_LOGO "AES"


/**
 * @brief Build (or rebuild) the UI on the terminal.
 * @ingroup ui
 */
extern void ui_term_build(void);

/**
 * @brief Update the sender station ID in the top of the terminal.
 * @ingroup ui
 *
 * @param id The station ID of the sender. A NULL will clear the sender line.
 */
extern void ui_term_sender_update(const char* id);

/**
 * @brief Update the status bar.
 */
extern void ui_term_status_update();


#ifdef __cplusplus
    }
#endif
#endif // _UI_TERM_H_