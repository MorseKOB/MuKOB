/**
 * MuKOB User Interface - On the display, rotory, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"
#include "config.h"
#include "display.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"

#include <string.h>

// TODO - Have these adjust based on the screen and font sizes

#define UI_DISP_TOP_FIXED_LINES 2
#define UI_DISP_BOTTOM_FIXED_LINES_INIT 5

// Top header and gap
#define UI_DISP_HEADER_COLOR_FG C16_YELLOW
#define UI_DISP_HEADER_COLOR_BG C16_BLUE
#define UI_DISP_HEADER_INFO_LINE 0
//#define UI_DISP_HEADER_GAP_LINE 1
#define UI_DISP_HEADER_CONNECTED_ICON_COL 0
#define UI_DISP_HEADER_SPEED_LABEL_COL 8
#define UI_DISP_HEADER_SPEED_VALUE_COL 10
#define UI_DISP_HEADER_WIRE_LABEL_COL 2
#define UI_DISP_HEADER_WIRE_VALUE_COL 4
#define UI_DISP_HEADER_SETUP_COL 20
#define UI_DISP_HEADER_MENU_COL 22

// Current sender line (at the top)
#define UI_DISP_SENDER_COLOR_FG C16_LT_BLUE
#define UI_DISP_SENDER_COLOR_BG C16_YELLOW
#define UI_DISP_SENDER_LINE 1

// Bottom status
#define UI_DISP_STATUS_COLOR_FG C16_YELLOW
#define UI_DISP_STATUS_COLOR_BG C16_BLUE
#define UI_DISP_STATUS_LINE 19
#define UI_DISP_STATUS_LOGO_COL 23
#define UI_DISP_STATUS_TIME_COL 9

static bool _displaying_code;

static void _header_fill_fixed() {
    text_color_pair_t cp;

    disp_get_text_colors(&cp);
    disp_set_text_colors(UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG);
    disp_line_clear(UI_DISP_HEADER_INFO_LINE, No_Paint);
    // disp_line_clear(UI_DISP_HEADER_GAP_LINE, No_Paint);
    // Fixed text
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_WIRE_LABEL_COL, "W:", false, No_Paint);
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SPEED_LABEL_COL, "S:", false, No_Paint);
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SETUP_COL, "\012\013", false, No_Paint); // Gear LF/RT
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_MENU_COL, "\014\015", false, No_Paint);  // Lines LF/RT
    // Paint and put the colors back
    disp_paint();
    disp_set_text_colors_cp(&cp);
}

static void _status_fill_fixed() {
    text_color_pair_t cp;

    disp_get_text_colors(&cp);
    disp_set_text_colors(UI_DISP_STATUS_COLOR_FG, UI_DISP_STATUS_COLOR_BG);
    disp_line_clear(UI_DISP_STATUS_LINE, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, 0, '\000', No_Paint); // Mu
    disp_string(UI_DISP_STATUS_LINE, 1, "KOB", false, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, UI_DISP_STATUS_LOGO_COL, '\177', No_Paint); // AES logo
    // Paint and put the colors back
    disp_paint();
    disp_set_text_colors_cp(&cp);
}

void ui_disp_build(void) {
    _displaying_code = false;
    disp_set_text_colors(C16_WHITE, C16_BLACK);
    disp_clear(Paint);
    scroll_area_define(UI_DISP_TOP_FIXED_LINES, UI_DISP_BOTTOM_FIXED_LINES_INIT);
    _header_fill_fixed();
    _status_fill_fixed();
    ui_disp_display_speed();
    ui_disp_display_wire();
    ui_disp_update_sender(NULL); // Build a blank sender line
    ui_disp_update_status();
}

void ui_disp_display_speed() {
    const config_t* config = config_current();
    ui_disp_update_speed(config->text_speed);
}

void ui_disp_display_wire() {
    const config_t* config = config_current();
    ui_disp_update_wire(config->wire);
}

void ui_disp_put_codetext(char* str) {
    if (!_displaying_code) {
        print_crlf(0, No_Paint);
        _displaying_code = true;
    }
    // If this is a '=' print a newline.
    if (strchr(str, '=')) {
        prints(str, No_Paint);
        print_crlf(0, Paint);
    }
    else {
        prints(str, Paint);
    }
}

void ui_disp_puts(char* str) {
    if (_displaying_code) {
        print_crlf(0, No_Paint);
        _displaying_code = false;
    }
    prints(str, Paint);
}

extern void ui_disp_update_connected_state(wire_connected_state_t state) {
    char* state_indicator = (WIRE_CONNECTED == state ? "\026\027" : "\024\025"); // Conn/Not-Conn LF/RT
    disp_string_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_CONNECTED_ICON_COL,
        state_indicator, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint);
}

void ui_disp_update_sender(const char* id) {
    text_color_pair_t cp;
    char buf[disp_info_columns() + 1];

    disp_get_text_colors(&cp);
    disp_set_text_colors(UI_DISP_SENDER_COLOR_FG, UI_DISP_SENDER_COLOR_BG);
    disp_line_clear(UI_DISP_SENDER_LINE, (id ? No_Paint : Paint));
    if (id) {
        snprintf(buf, sizeof(buf) - 1, ">%s", id);
        disp_string(UI_DISP_SENDER_LINE, 0, buf, false, Paint);
    }
    disp_set_text_colors_cp(&cp);
}

void ui_disp_update_speed(uint16_t speed) {
    char buf[5];

    snprintf(buf, sizeof(buf) - 1, "%-2hd", speed);
    disp_string_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SPEED_VALUE_COL, buf, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint);
}

void ui_disp_update_status() {
    // Put the current time in the center
    char buf[10];
    datetime_t now;

    rtc_get_datetime(&now);
    strdatetime(buf, 9, &now, SDTC_TIME_2CHAR_HOUR | SDTC_TIME_AMPM);
    disp_string_color(UI_DISP_STATUS_LINE, UI_DISP_STATUS_TIME_COL, buf, UI_DISP_STATUS_COLOR_FG, UI_DISP_STATUS_COLOR_BG, Paint);
}

void ui_disp_update_wire(uint16_t wire) {
    char buf[5];

    snprintf(buf, sizeof(buf) - 1, "%-3hd", wire);
    disp_string_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_WIRE_VALUE_COL, buf, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint);
}
