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
#include "term.h"
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

void test_disp_show_full_scroll_barberpoll() {
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

void test_disp_show_half_width_scroll_barberpoll() {
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

void test_ili9341_show_scroll() {
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

void test_disp_show_mukob_head_foot() {
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

void test_term_color_chart() {
    term_cursor_on(false);
    term_clear();
    term_cursor_moveto(2,0);
    term_color_default();
    printf("\nRED: "); term_color_fg(TERM_CHR_COLOR_RED); printf("The quick brown fox...");
    term_color_default();
    printf("\nGREEN: "); term_color_fg(TERM_CHR_COLOR_GREEN); printf("The quick brown fox...");
    term_color_default();
    printf("\nYELLOW: "); term_color_fg(TERM_CHR_COLOR_YELLOW); printf("The quick brown fox...");
    term_color_default();
    printf("\nBLUE: "); term_color_fg(TERM_CHR_COLOR_BLUE); printf("The quick brown fox...");
    term_color_default();
    printf("\nMAGENTA: "); term_color_fg(TERM_CHR_COLOR_MAGENTA); printf("The quick brown fox...");
    term_color_default();
    printf("\nCYAN: "); term_color_fg(TERM_CHR_COLOR_CYAN); printf("The quick brown fox...");
    term_color_default();
    printf("\nWHITE: "); term_color_fg(TERM_CHR_COLOR_WHITE); printf("The quick brown fox...");

    term_color_default();
    printf("\nBRIGHT RED: "); term_color_fg(TERM_CHR_COLOR_BR_RED); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT GREEN: "); term_color_fg(TERM_CHR_COLOR_BR_GREEN); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT YELLOW: "); term_color_fg(TERM_CHR_COLOR_BR_YELLOW); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT BLUE: "); term_color_fg(TERM_CHR_COLOR_BR_BLUE); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT MAGENTA: "); term_color_fg(TERM_CHR_COLOR_BR_MAGENTA); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT CYAN: "); term_color_fg(TERM_CHR_COLOR_BR_CYAN); printf("The quick brown fox...");
    term_color_default();
    printf("\nBRIGHT WHITE: "); term_color_fg(TERM_CHR_COLOR_BR_WHITE); printf("The quick brown fox...");

    // term_cursor_on(true);
}

void test_term_screen_page_size() {
    // Set the size
    term_set_size(48, 132);
    term_clear();
    // Print 1-0... across the screen
    int text_color = 0;
    term_color_fg(text_color);
    for (int col = 1; col <= 132; col++) {
        if ((col - 1) % 10 == 0) {
            text_color++;
            term_color_fg(text_color);
        }
        putchar('0' + (col % 10));
    }
    putchar('\n');
    // Print a diagonal
    term_color_default();
    for (int line = 1; line <= 48; line++) {
        int col = (line - 1) * 3;
        int ac = (col < 126 ? col : 126);
        term_cursor_moveto(line, ac);
        printf("%d,%d", line, col);
    }
}
