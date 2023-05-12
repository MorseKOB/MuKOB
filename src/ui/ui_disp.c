/**
 * MuKOB User Interface - On the display, rotary, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"
#include "config.h"
#include "display.h"
#include "font.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"

#include <stdlib.h>
#include <string.h>

// TODO - Have these adjust based on the screen and font sizes

#define UI_DISP_TOP_FIXED_LINES 2
#define UI_DISP_BOTTOM_FIXED_LINES 1

// Top header and gap
#define UI_DISP_HEADER_COLOR_FG C16_YELLOW
#define UI_DISP_HEADER_COLOR_BG C16_BLUE
#define UI_DISP_HEADER_INFO_LINE 0
#define UI_DISP_HEADER_CONNECTED_ICON_COL 0
#define UI_DISP_HEADER_SPEED_LABEL_COL 8
#define UI_DISP_HEADER_SPEED_VALUE_COL 10
#define UI_DISP_HEADER_WIRE_LABEL_COL 2
#define UI_DISP_HEADER_WIRE_VALUE_COL 4
// Header icon locations
#define UI_DISP_HEADER_CLOSER_COL 13
#define UI_DISP_HEADER_LOOP_COL 16
#define UI_DISP_HEADER_WIFI_COL 19
#define UI_DISP_HEADER_SETUP_COL 20
#define UI_DISP_HEADER_MENU_COL 22

// Current sender line (at the top)
#define UI_DISP_SENDER_COLOR_FG C16_LT_BLUE
#define UI_DISP_SENDER_COLOR_BG C16_YELLOW
#define UI_DISP_SENDER_LINE 1

// Active Stations lines (below the code area)
#define UI_DISP_STATIONS_COLOR_FG C16_MAGENTA
#define UI_DISP_STATIONS_COLOR_BG C16_BLACK
#define UI_DISP_STATIONS_LINES_MAX 4

// Bottom status
#define UI_DISP_STATUS_COLOR_FG C16_YELLOW
#define UI_DISP_STATUS_COLOR_BG C16_BLUE
#define UI_DISP_STATUS_LINE 19
#define UI_DISP_STATUS_LOGO_COL 23
#define UI_DISP_STATUS_TIME_COL 9

static uint16_t _active_stations_lines;
static bool _code_displaying;
static kob_status_t _kob_status;

static void _header_fill_fixed() {
    text_color_pair_t cp;

    disp_text_colors_get(&cp);
    disp_text_colors_set(UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG);
    disp_line_clear(UI_DISP_HEADER_INFO_LINE, No_Paint);
    // disp_line_clear(UI_DISP_HEADER_GAP_LINE, No_Paint);
    // Fixed text
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_WIRE_LABEL_COL, "W:", false, No_Paint);
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SPEED_LABEL_COL, "S:", false, No_Paint);
    disp_char(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_WIFI_COL, WIFI_CONNECTED_CHR, No_Paint); // WiFi
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SETUP_COL, "\012\013", false, No_Paint); // Gear LF/RT
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_MENU_COL, "\014\015", false, No_Paint);  // Lines LF/RT
    // Paint and put the colors back
    disp_paint();
    disp_text_colors_cp_set(&cp);
}

static void _status_fill_fixed() {
    text_color_pair_t cp;

    disp_text_colors_get(&cp);
    disp_text_colors_set(UI_DISP_STATUS_COLOR_FG, UI_DISP_STATUS_COLOR_BG);
    disp_line_clear(UI_DISP_STATUS_LINE, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, 0, '\000', No_Paint); // Mu
    disp_string(UI_DISP_STATUS_LINE, 1, "KOB", false, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, UI_DISP_STATUS_LOGO_COL, '\177', No_Paint); // AES logo
    // Paint and put the colors back
    disp_paint();
    disp_text_colors_cp_set(&cp);
}

void ui_disp_build(void) {
    _code_displaying = false;
    disp_text_colors_set(C16_LT_GREEN, C16_BLACK);
    disp_clear(Paint);
    disp_scroll_area_define(UI_DISP_TOP_FIXED_LINES, (UI_DISP_BOTTOM_FIXED_LINES + _active_stations_lines));
    _header_fill_fixed();
    _status_fill_fixed();
    ui_disp_update_circuit_closed(_kob_status.circuit_closed);
    ui_disp_update_key_closed(_kob_status.key_closed);
    ui_disp_display_speed();
    ui_disp_display_wire();
    ui_disp_update_sender(NULL); // Build a blank sender line
    ui_disp_update_status();
}

void ui_disp_display_speed() {
    const config_t* cfg = config_current();
    ui_disp_update_speed(cfg->text_speed);
}

void ui_disp_display_wire() {
    const config_t* cfg = config_current();
    ui_disp_update_wire(cfg->wire);
}

void ui_disp_put_codetext(char* str) {
    if (!_code_displaying) {
        disp_print_crlf(0, No_Paint);
        _code_displaying = true;
    }
    // If this is a '=' print a newline.
    if (strchr(str, '=')) {
        disp_prints(str, No_Paint);
        disp_print_crlf(0, Paint);
    }
    else {
        disp_prints(str, Paint);
    }
}

void ui_disp_puts(char* str) {
    if (_code_displaying) {
        disp_print_crlf(0, No_Paint);
        _code_displaying = false;
    }
    disp_prints(str, Paint);
}

void ui_disp_update_circuit_closed(bool closed) {
    char indicator = (closed ? LOOP_CLOSED_CHR : LOOP_OPEN_CHR);
    disp_char_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_LOOP_COL, indicator, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint); // Loop
}

void ui_disp_update_connected_state(wire_connected_state_t state) {
    char* state_indicator = (WIRE_CONNECTED == state ? "\026\027" : "\024\025"); // Conn/Not-Conn LF/RT
    disp_string_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_CONNECTED_ICON_COL,
        state_indicator, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint);
}

void ui_disp_update_key_closed(bool closed) {
    char indicator_l = (closed ? CLOSER_CLOSED_LG_L_CHR : CLOSER_OPEN_LG_L_CHR);
    char indicator_r = (closed ? CLOSER_CLOSED_LG_R_CHR : CLOSER_OPEN_LG_R_CHR);
    disp_char_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_CLOSER_COL, indicator_l, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint); // Closer Left
    disp_char_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_CLOSER_COL + 1, indicator_r, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint); // Closer Right
}

void ui_disp_update_kob_status(const kob_status_t* kob_status) {
    if (kob_status->circuit_closed != _kob_status.circuit_closed) {
        _kob_status.circuit_closed = kob_status->circuit_closed;
        ui_disp_update_circuit_closed(kob_status->circuit_closed);
    }
    if (kob_status->key_closed != _kob_status.key_closed) {
        _kob_status.key_closed = kob_status->key_closed;
        ui_disp_update_key_closed(kob_status->key_closed);
    }
    if (kob_status->sounder_energized != _kob_status.sounder_energized) {
        _kob_status.sounder_energized = kob_status->sounder_energized;
        // ui_disp_update_sounder_energized(kob_status.sounder_energized);
    }
}

void ui_disp_update_sender(const char* id) {
    text_color_pair_t cp;
    char buf[disp_info_columns() + 1];

    // Put a newline in the code window
    disp_print_crlf(0, Paint);
    disp_text_colors_get(&cp);
    disp_text_colors_set(UI_DISP_SENDER_COLOR_FG, UI_DISP_SENDER_COLOR_BG);
    disp_line_clear(UI_DISP_SENDER_LINE, (id ? No_Paint : Paint));
    if (id) {
        snprintf(buf, sizeof(buf) - 1, ">%s", id);
        disp_string(UI_DISP_SENDER_LINE, 0, buf, false, Paint);
    }
    disp_text_colors_cp_set(&cp);
}

void ui_disp_update_speed(uint16_t speed) {
    char buf[5];

    snprintf(buf, sizeof(buf) - 1, "%-2hd", speed);
    disp_string_color(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SPEED_VALUE_COL, buf, UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG, Paint);
}

void ui_disp_update_stations(const mk_station_id_t** stations, int count) {
    // Erase the current stations area
    for (int j = (UI_DISP_STATUS_LINE - 1); j > (UI_DISP_STATUS_LINE - (_active_stations_lines + 1)); j--) {
        disp_line_clear(j, No_Paint);
    }
    // How many lines to display
    int lines = (count <= UI_DISP_STATIONS_LINES_MAX ? count : UI_DISP_STATIONS_LINES_MAX);
    // Set the scroll area if needed (if it needs to be different from the current)
    if ((lines + UI_DISP_BOTTOM_FIXED_LINES) != disp_info_fixed_bottom_lines()) {
        disp_scroll_area_define(UI_DISP_TOP_FIXED_LINES, (lines + UI_DISP_BOTTOM_FIXED_LINES));
    }
    _active_stations_lines = lines;
    // Display the stations
    uint16_t line = UI_DISP_STATUS_LINE - lines;
    uint16_t cols = disp_info_columns();
    char buf[cols];
    for (int i = 0; i < lines; i++) {
        const mk_station_id_t * station = *(stations + i);
        snprintf(buf, cols, "%s", station->id);
        disp_string_color(line++, 0, buf, UI_DISP_STATIONS_COLOR_FG, UI_DISP_STATIONS_COLOR_BG, No_Paint);
    }
    disp_paint();
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
