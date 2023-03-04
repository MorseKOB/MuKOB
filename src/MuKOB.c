/**
 * MuKOB main application.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
//#include <stdio.h>
//#include <stdarg.h>
#include "pico/binary_info.h"
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/datetime.h"

#include "system_defs.h"
// (some defines in the above are used in the following, so must be included first)
#include "display.h"
#include "display_ili9341.h"
#include "font.h"
#include "mkwire.h"
#include "mkboard.h"
#include "net.h"
#include "test.h"
#include "term.h"
#include "util.h"

#undef putc
#undef putchar

static int32_t say_hi[] = {100,100,100,100,100,100,100,100,250,100,100,100,0}; // 'H' (....) 'I' (..)

int main()
{
    char buf[128];
    datetime_t now;

    // // useful information for picotool
    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    // bi_decl(bi_program_description("Micro version of MorseKOB"));
    board_init();

    if (option_value(OPTION_DEBUG)) {
        buzzer_beep(250);
    }
    led_on_off(say_hi);

    // Try to connect to the MKOB Server wire 108
    // mkwire_init("192.168.68.85", MKOBSERVER_PORT_DEFAULT, "ES, Ed, WA (MuKOB)");
    // mkwire_connect(90); // Test wire
    // sleep_ms(15000);

    // Show (test) the RTC datetime to string function
    test_strdatetime();
    sleep_ms(3000);

    // Test the config new & free
    test_config_new_free();

    // Set up the display
    disp_clear(Paint);
    disp_set_text_colors(C16_BR_WHITE, C16_BLACK);

    // disp_font_test();
    // sleep_ms(2000);

    // ili9341_colors_show();
    // sleep_ms(2000);

    // disp_c16_color_chart();
    // sleep_ms(2000);
    // disp_clear(Paint);

    cursor_show(false);
    cursor_home();
    printf_disp(Paint, "This text is on the main\n(base) screen.\n");

    rtc_get_datetime(&now);
    strdatetime(buf, 127, &now, SDTC_LONG_TXT_ON | SDTC_DATE | SDTC_TIME);
    printf_disp(Paint, "This was printed at: %s", buf);
    sleep_ms(1000);
    // Test terminal input callback
    printf("Testing terminal input and notification. Type a character: ");
    char tti = test_term_notify_on_input(10000); // Wait up to 10 seconds for a character
    if (tti >= 0) {
        printf("\n You typed '%c'\n", tti);
    }
    else {
        printf("\n Either you didn't type anything, or the input chain didn't work.\n");
    }
    term_input_buf_clear();
    sleep_ms(3000);

    colorbyte_t color = 0;
    while(true) {
        options_read();  // Re-read the option switches

        led_on_off(say_hi);

        // Some terminal tests
        for (int i = 0; i < 3; i++) {
            test_term_color_chart();
            sleep_ms(200);
        }
        sleep_ms(1000);
        test_term_screen_page_size();
        sleep_ms(3000);
        test_term_scroll_area();
        sleep_ms(5000);

        bool input_overflow = false;
        while (term_input_available()) {
            int i = term_getc();
            if (i < 0) {
                printf("term_input_available() was true, but term_getc() said not.\n");
            }
            else {
                if (i == '0') {
                    printf("Set color back to 0.\n");
                    color = 0;
                }
                else {
                    putchar_raw(i);
                }
            }
            input_overflow |= term_input_overflow();
        }
        if (input_overflow) {
            warn_printf("\nInput data from the terminal was lost.\n");
        }
        color++;
        uint8_t fgc = fg_from_cb(color);
        uint8_t bgc = bg_from_cb(color);
        if (fgc == bgc) {
            continue;
        }
        screen_new();
        disp_set_text_colors(fgc, bgc);
        disp_clear(Paint);
        cursor_set(19,0);
        test_disp_show_full_scroll_barberpoll();

        // Test creating a new (sub) screen and writing to it
        screen_new();
        printf_disp(No_Paint, "This is on a new (sub)\nscreen!\n\n");
        printf_disp(Paint, "Then it will delay. At\nthe end, pop the\nsub-screen off -\nrestoring the previous screen.");
        sleep_ms(2000);

        test_disp_show_mukob_head_foot();
        disp_set_text_colors(C16_LT_GREEN, C16_BLACK);
        disp_string(12, 0, "098765432109876543210987", false, true);
        disp_string(13, 0, "A 1 B 2 C 3 D 4 E 5 F 6 ", false, true);
        disp_string(14, 0, " A 1 B 2 C 3 D 4 E 5 F 6", false, true);
        sleep_ms(1000);
        cursor_home();
        // void test_ili9341_show_scroll();
        test_disp_show_half_width_scroll_barberpoll();
        sleep_ms(1000);
        disp_clear(Paint);
        screen_close();
        sleep_ms(1000);
        disp_clear(Paint);
        screen_close();
        sleep_ms(1000);
    }

    return 0;
}
