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
}

alarm_id_t _test_term_notify_on_input_to_id;
static char _input;
static volatile bool _test_term_notify_on_input_called;
static void _test_term_notify_on_input(void) {
    // Function that is registered.
    _input = '\000';
    if (term_input_available()) {
        _input = term_getc();
    }
    _test_term_notify_on_input_called = true;
}
static int64_t _test_term_notify_on_input_to(alarm_id_t id, void* not_used) {
    cancel_alarm(id);
    _test_term_notify_on_input_to_id = 0;
    error_printf("\nTEST - test_term_notify_on_input timed out.\n");
    _input = -1;
    term_notify_on_input(NULL);
    _test_term_notify_on_input_called = true; // Say this was called, but the char is '-1'

    return (0); // Don't reschedule this timer
}
char test_term_notify_on_input(uint32_t timeout) {
    uint32_t time_waited = 0; // A secondary timeout
    _test_term_notify_on_input_called = false;
    _input = -1; // Return -1 if we timeout
    _test_term_notify_on_input_to_id = add_alarm_in_ms(timeout, _test_term_notify_on_input_to, NULL, true);
    // Register our callback and wait...
    term_notify_on_input(_test_term_notify_on_input);

    while (!_test_term_notify_on_input_called && time_waited < timeout + 250) {
        sleep_ms(10);
        time_waited += 10;
        if (time_waited % 500 == 0) {
            putchar('.');
        }
    }
    if (_test_term_notify_on_input_to_id != 0) {
        cancel_alarm(_test_term_notify_on_input_to_id);
    }
    term_notify_on_input(NULL);

    return (_input);
}

void test_term_scroll_area() {
    term_reset();
    term_set_type(VT_510_TYPE_SPEC, VT_510_ID_SPEC);
    term_set_title("μKOB v0.1");
    //term_set_title("MuKOB v0.1");
    // Set the screen/page to a known size
    term_set_size(24, 80);
    // Put some text in the top fixed area
    term_cursor_moveto(1, 1);
    printf("TOP-FIXED-LEFT");
    term_cursor_moveto(1, 65);
    printf("TOP-FIXED-RIGHT");
    term_cursor_moveto(2, 1);
    printf("TOP-FIXED-LEFT-2");
    term_cursor_moveto(2, 63);
    printf("TOP-FIXED-RIGHT-2");
    // Put some text in the bottom fixed area
    term_cursor_moveto(14, 1);
    printf("BOTTOM-FIXED-LEFT-14");
    term_cursor_moveto(14, 59);
    printf("BOTTOM-FIXED-RIGHT-14");
    term_cursor_moveto(25, 1);
    printf("BOTTOM-FIXED-LEFT-24");
    term_cursor_moveto(25, 59);
    printf("BOTTOM-FIXED-RIGHT-24");
    term_cursor_moveto(2, 30);
    printf("Iteration    of 80");
    term_cursor_moveto(1, 35);
    printf("Cursor:");
    // Print a lines down the right side
    for (int l = 3; l < 14; l++) {
        term_cursor_moveto(l, 73);
        printf("Line %2d", l);
    }
    // Set a top fixed of 2 and a scroll area of 10
    term_set_margin_top_bottom(3, 13);
    term_cursor_on(false);
    // Now, print multiple lines in the scroll area
    // Print a scrolling barber pole
    char ca = 0;
    for (int i = 0; i < 20; i++) {
        for (int col = 0; col < 80; col++) {
            char c = '@' + ca + col;
            c &= 0x1F;
            c |= 0x40;
            putchar(c);
            term_cursor_save();
            scr_position_t curpos = term_get_cursor_position();
            term_set_origin_mode(TERM_OM_UPPER_LEFT);
            term_cursor_moveto(1, 43);
            printf("%hd,%-3hd", curpos.line, curpos.column);
            term_set_origin_mode(TERM_OM_IN_MARGINS);
            term_cursor_restore();
        }
        term_cursor_save();
        term_set_origin_mode(TERM_OM_UPPER_LEFT);
        term_cursor_moveto(2, 40);
        printf("%2d", i+1);
        term_set_origin_mode(TERM_OM_IN_MARGINS);
        term_cursor_restore();
        sleep_ms(80);
        ca++;
    }
    term_set_origin_mode(TERM_OM_UPPER_LEFT); // Allow cursor in full screen
    term_set_margin_top_bottom(0,0);
    sleep_ms(2000);
    term_cursor_moveto(8, 40);
    term_erase_line();
    term_cursor_up_1();
    term_erase_bol();
    term_cursor_down(2);
    term_erase_eol();
    sleep_ms(2000);
    term_cursor_moveto(25, 0);
    term_cursor_on(true);
}

void test_term_screen_page_size() {
    // Set the size
    term_set_size(25, 80);
    term_clear();
    // Print 1-0... across the screen
    int text_color = 0;
    term_color_fg(text_color);
    for (int col = 1; col <= 80; col++) {
        if ((col - 1) % 10 == 0) {
            text_color++;
            term_color_fg(text_color);
        }
        putchar('0' + (col % 10));
    }
    putchar('\n');
    // Print a diagonal
    term_color_default();
    for (int line = 1; line <= 25; line++) {
        int col = (line - 1) * 4;
        int ac = (col < 75 ? col : 75);
        term_cursor_moveto(line, ac);
        printf("%2d,%-2d", line, col);
    }
    // Print the term ID in the center
    char buf[64];
    term_cursor_moveto(12, 35);
    if (term_get_id_info(VT_220_ID_SPEC, buf, 63) > 1) {
        printf("Term ID: 'ESC%s'", buf+1);
    }
    else {
        printf("No Term ID returned");
    }
    term_cursor_moveto(13, 35);
    if (term_get_name(buf, 63) > 1) {
        printf("Term Name: '%s'", buf);
    }
    else {
        printf("No Term Name returned");
    }
}
