/**
 * MuKOB User Interface - On the display, rotory, touch.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "ui_disp.h"
#include "display.h"
#include "util.h"
#include "hardware/rtc.h"
#include "pico/printf.h"

// TODO - Have these adjust based on the screen and font sizes

#define UI_DISP_TOP_FIXED_LINES 3
#define UI_DISP_BOTTOM_FIXED_LINES_INIT 5

// Top header and gap
#define UI_DISP_HEADER_COLOR_FG C16_YELLOW
#define UI_DISP_HEADER_COLOR_BG C16_BLUE
#define UI_DISP_HEADER_INFO_LINE 0
#define UI_DISP_HEADER_GAP_LINE 1
#define UI_DISP_HEADER_WIRE_LABEL_COL 2
#define UI_DISP_HEADER_SPEED_LABEL_COL 8

// Current sender line (at the top)
#define UI_DISP_SENDER_COLOR_FG C16_LT_BLUE
#define UI_DISP_SENDER_COLOR_BG C16_YELLOW
#define UI_DISP_SENDER_LINE 2

// Bottom status
#define UI_DISP_STATUS_COLOR_FG C16_YELLOW
#define UI_DISP_STATUS_COLOR_BG C16_BLUE
#define UI_DISP_STATUS_LINE 19
#define UI_DISP_STATUS_LOGO_COL 23
#define UI_DISP_STATUS_TIME_COL 8

static void _header_fill_fixed() {
    text_color_pair_t cp;

    disp_get_text_colors(&cp);
    disp_set_text_colors(UI_DISP_HEADER_COLOR_FG, UI_DISP_HEADER_COLOR_BG);
    disp_line_clear(UI_DISP_HEADER_INFO_LINE, No_Paint);
    disp_line_clear(UI_DISP_HEADER_GAP_LINE, No_Paint);
    // Fixed text
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_WIRE_LABEL_COL, "W:", false, No_Paint);
    disp_string(UI_DISP_HEADER_INFO_LINE, UI_DISP_HEADER_SPEED_LABEL_COL, "S:", false, No_Paint);
    // Put the colors back and paint
    disp_set_text_colors_cp(&cp);
    disp_paint();
}

static void _status_fill_fixed() {
    text_color_pair_t cp;

    disp_get_text_colors(&cp);
    disp_set_text_colors(UI_DISP_STATUS_COLOR_FG, UI_DISP_STATUS_COLOR_BG);
    disp_line_clear(UI_DISP_STATUS_LINE, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, 0, '\000', No_Paint); // Mu
    disp_string(UI_DISP_STATUS_LINE, 1, "KOB", false, No_Paint);
    disp_char(UI_DISP_STATUS_LINE, UI_DISP_STATUS_LOGO_COL, '\177', No_Paint);
    disp_set_text_colors_cp(&cp);
    disp_paint();
}

void ui_build_disp(void) {
    disp_set_text_colors(C16_WHITE, C16_BLACK);
    disp_clear(Paint);
    scroll_area_define(UI_DISP_TOP_FIXED_LINES, UI_DISP_BOTTOM_FIXED_LINES_INIT);
    _header_fill_fixed();
    _status_fill_fixed();
    ui_disp_sender_update(NULL); // Build a blank sender line
    ui_disp_status_update();
}

void ui_disp_sender_update(const char* id) {
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

void ui_disp_status_update() {
    // Put the current time in the center
    char buf[10];
    datetime_t now;

    rtc_get_datetime(&now);
    strdatetime(buf, 9, &now, SDTC_TIME_2CHAR_HOUR | SDTC_TIME_AMPM);
    disp_string_color(UI_DISP_STATUS_LINE, UI_DISP_STATUS_TIME_COL, buf, UI_DISP_STATUS_COLOR_FG, UI_DISP_STATUS_COLOR_BG, Paint);
}
