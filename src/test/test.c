/**
 * @brief Test, demo, and debugging routines.
 * @ingroup test
 *
 * Many of these aren't actually tests. Rather, they tend to be routines
 * that display patterns, send things to the terminal, get rotory control
 * values, etc., that can be helpful for seeing, demo'ing, and debugging
 * functionality.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "test.h"
#include "system_defs.h"
#include "display.h"
#include "ili9341_spi.h"
#include "mukboard.h"
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/datetime.h"

void test_error_printf() {
    error_printf("Test of printing an error: %d.\n", 15u);
}

void test_show_full_scroll_barberpoll() {
    scroll_area_define(0, 0);
    char ca = 0;
    for (int i = 0; i < 80; i++) {
        for (int col = 0; col < disp_info_columns(); col++) {
            char c = '@' + ca + col;
            c &= 0x1F;
            c |= 0x40;
            printc(c, No_Paint);
        }
        disp_paint();
        ca++;
    }
}

void test_show_half_width_scroll_barberpoll() {
    uint8_t ca = 0;
    for (int i = 0; i < 100; i++) {
        for (int col = 0; col < 16; col++) {
            char c = '@' + ca + col;
            c &= 0x1F;
            c |= 0x40;
            printc(c, Paint);
        }
        print_crlf(0, Paint);
        ca++;
    }
}

void test_show_ili9341_scroll() {
    cursor_home();
    // Test scroll with a window
    uint8_t ca = 0;
    for (int i = 0; i < 17; i++) {
        for (int col = 0; col < 16; col++) {
            char c = '@' + ca + col;
            c &= 0x1F;
            c |= 0x40;
            disp_char(i + 2, col, c, No_Paint);
        }
        for (int col = 16; col < 24; col++) {
            char c = '0' + (col - 16);
            disp_char(i + 2, col, c, No_Paint);
        }
        disp_paint();
        ca++;
    }
    // Now scroll the ILI9341 (10 times)
    for (int i = 0; i < 10; i++) {
        for (int ss = 0; ss < 320; ss += 8) {
            ili9341_scroll_set_start(ss);
        }
    }
}

void test_show_mukob_head_foot() {
    datetime_t t;
    rtc_get_datetime(&t);
    scroll_area_define(2, 1);
    disp_set_text_colors(C16_YELLOW, C16_BLUE);
    char buf[25];
    sprintf(buf, " KOB      %2d:%2d        \177", t.hour, t.min);
    disp_string(disp_info_lines() - 1, 0, buf, false, true);
    disp_char(disp_info_lines() - 1, 0, '\000', true); // Put the 'mu' in front of the 'KOB'
    disp_string(0, 0, "\024\025W:108 S:25 \022\023 \016 \002 \012\013\014\015", false, true);
    disp_set_text_colors(C16_BLUE, C16_YELLOW);
    disp_string(1, 0, "ES, Ed, WA 1234567890123", false, true);
}
