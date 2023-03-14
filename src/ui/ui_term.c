/**
 * MuKOB User Interface - On the terminal.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_term.h"
#include "term.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"

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

// Labels
#define UI_TERM_WIRE_LABEL "Wire:"
#define UI_TERM_SPEED_LABEL "Speed:"
#define AES_LOGO "AES"

static void _header_fill_fixed() {
    term_color_fg(UI_TERM_HEADER_COLOR_FG);
    term_color_bg(UI_TERM_HEADER_COLOR_BG);
    term_cursor_save();
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, 1);
    term_erase_line();
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_WIRE_LABEL_COL);
    printf("%s", UI_TERM_WIRE_LABEL);
    term_cursor_moveto(UI_TERM_HEADER_INFO_LINE, UI_TERM_HEADER_SPEED_LABEL_COL);
    printf("%s", UI_TERM_SPEED_LABEL);
    term_color_default();
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

static void _status_fill_fixed() {
    term_cursor_save();
    term_color_fg(UI_TERM_STATUS_COLOR_FG);
    term_color_bg(UI_TERM_STATUS_COLOR_BG);
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_STATUS_LINE, 1);
    term_erase_line();
    printf("%s", UI_TERM_NAME_VERSION);
    term_cursor_moveto(UI_TERM_STATUS_LINE, UI_TERM_STATUS_LOGO_COL);
    printf("%s", AES_LOGO);
    term_color_default();
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

static void _term_init() {
    term_reset();
    term_color_default();
    term_set_type(VT_510_TYPE_SPEC, VT_510_ID_SPEC);
    term_set_title(UI_TERM_NAME_VERSION);
    term_set_size(UI_TERM_LINES, UI_TERM_COLUMNS);
    term_clear();
    term_set_margin_top_bottom(UI_TERM_SCROLL_START_LINE, UI_TERM_SCROLL_END_LINE);
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_clear();
    term_cursor_on(false);
}

void ui_term_build(void) {
    _term_init();
    _header_fill_fixed();
    _status_fill_fixed();
    ui_term_sender_update(NULL); // Build a blank sender line
    ui_term_status_update();
}

void ui_term_sender_update(const char* id) {
    char buf[UI_TERM_COLUMNS + 1];

    term_color_fg(UI_TERM_SENDER_COLOR_FG);
    term_color_bg(UI_TERM_SENDER_COLOR_BG);
    term_cursor_save();
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_SENDER_LINE, 1);
    term_erase_line();
    if (id) {
        snprintf(buf, sizeof(buf) - 1, ">%s", id);
        printf("%s", buf);
    }
    term_color_default();
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}

void ui_term_status_update() {
    // Put the current time in the center
    char buf[10];
    datetime_t now;

    rtc_get_datetime(&now);
    strdatetime(buf, 9, &now, SDTC_TIME_2CHAR_HOUR | SDTC_TIME_AMPM);
    term_color_fg(UI_TERM_STATUS_COLOR_FG);
    term_color_bg(UI_TERM_STATUS_COLOR_BG);
    term_cursor_save();
    term_set_origin_mode(TERM_OM_UPPER_LEFT);
    term_cursor_moveto(UI_TERM_STATUS_LINE, UI_TERM_STATUS_TIME_COL);
    printf("%s", buf);
    term_color_default();
    term_set_origin_mode(TERM_OM_IN_MARGINS);
    term_cursor_restore();
}
