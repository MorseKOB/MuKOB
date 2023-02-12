/**
 * MuKOB main application.
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
*/
#include <stdio.h>
#include <stdarg.h>

#include "system_defs.h"
// (some defines in the above are used in the following, so must be included first)
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/datetime.h"
#include "net.h"
#include "mukboard.h"
#include "display_ili9341.h"
#include "mkwire.h"

void _mk_printf(const char* prefix, const char* format, ...) {
    char buf[1024];
    datetime_t t;
    rtc_get_datetime(&t);
    va_list xArgs;
    va_start(xArgs, format);
    vsnprintf(buf, sizeof(buf), format, xArgs);
    va_end(xArgs);
    fprintf(stdout, "%02d-%02d-%04d %02d:%02d:%02d - %s: %s", 
        t.month, t.day, t.year, t.hour, t.min, t.sec, prefix, buf);
    fflush(stdout);
}

void debug_printf(const char* format, ...) {
    if (option_value(OPTION_DEBUG)) {
        char buf[1024];
        datetime_t t;
        rtc_get_datetime(&t);
        va_list xArgs;
        va_start(xArgs, format);
        vsnprintf(buf, sizeof(buf), format, xArgs);
        va_end(xArgs);
        fprintf(stdout, "%02d-%02d-%04d %02d:%02d:%02d - DEBUG: %s",
            t.month, t.day, t.year, t.hour, t.min, t.sec, buf);
        fflush(stdout);
    }
}

void error_printf(const char* format, ...) {
    char buf[1024];
    datetime_t t;
    rtc_get_datetime(&t);
    va_list xArgs;
    va_start(xArgs, format);
    vsnprintf(buf, sizeof(buf), format, xArgs);
    va_end(xArgs);
    fprintf(stderr, "\033[91m%02d-%02d-%04d %02d:%02d:%02d - ERROR: %s\033[0m",
        t.month, t.day, t.year, t.hour, t.min, t.sec, buf);
    fflush(stderr);
}

void info_printf(const char* format, ...) {
    char buf[1024];
    datetime_t t;
    rtc_get_datetime(&t);
    va_list xArgs;
    va_start(xArgs, format);
    vsnprintf(buf, sizeof(buf), format, xArgs);
    va_end(xArgs);
    fprintf(stdout, "%02d-%02d-%04d %02d:%02d:%02d - INFO: %s",
        t.month, t.day, t.year, t.hour, t.min, t.sec, buf);
    fflush(stdout);
}

void warn_printf(const char* format, ...) {
    char buf[1024];
    datetime_t t;
    rtc_get_datetime(&t);
    va_list xArgs;
    va_start(xArgs, format);
    vsnprintf(buf, sizeof(buf), format, xArgs);
    va_end(xArgs);
    fprintf(stderr, "%02d-%02d-%04d %02d:%02d:%02d - WARN: %s",
        t.month, t.day, t.year, t.hour, t.min, t.sec, buf);
    fflush(stderr);
}

int say_hi[] = {100,100,100,100,100,100,100,100,250,100,100,100,0}; // 'H' (....) 'I' (..)

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    puts("Alarm occurred.");
    return 0;
}

int main()
{
    // // useful information for picotool
    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    // bi_decl(bi_program_description("Micro version of MorseKOB"));
    board_init();

    error_printf("Test of printing an error.\n");
    sleep_ms(2000);

    if (option_value(OPTION_DEBUG)) {
        buzzer_beep(250);
    }

    // Try to connect to the MKOB Server wire 108
    mkwire_init("192.168.68.85", MKOBSERVER_PORT_DEFAULT, "ES, Ed, WA (MuKOB)");
    mkwire_connect(90); // Test wire
    sleep_ms(15000);

    uint8_t color = 0;
    while(true) {
        options_read();  // Re-read the option switches
        color++;
        uint8_t fgc = fg_from_cb(color);
        uint8_t bgc = bg_from_cb(color);
        if (fgc == bgc) {
            continue;
        }
        // Obtain current time
        // `time()` returns the current time of the system as a `time_t` value
        time_t now;
        time(&now);
        // Convert to local time format and print to stdout
        printf("Today is %s", ctime(&now));

        disp_clear(true);
        sleep_ms(500);
        disp_set_text_colors(fg_from_cb(color), bg_from_cb(color));
        disp_font_test();
        sleep_ms(1000);
        //led_on_off(say_hi);

        disp_clear(true);
        margins_set(0, 0, DISP_CHAR_ROWS - 1, DISP_CHAR_COLS - 1);
        char ca = 0;
        for (int i = 0; i < 1 * DISP_CHAR_COLS * DISP_CHAR_ROWS; i++) {
            for (int col = 0; col < DISP_CHAR_COLS; col++) {
                char c = '@' + ca + col;
                c &= 0x1F;
                c |= 0x40;
                printc(c, false);
            }
            disp_paint();
            ca++;
        }

        // disp_clear(true);
        // // VGA Color Test
        // disp_set_text_colors(C16_BR_WHITE, C16_BLACK);
        // for (uint8_t i = 0; i < 16; i++) {
        //     char c = '0'+i;
        //     if (c > '9')
        //         c += 7;
        //     disp_char(4, 2 * i, c, true);
        //     disp_char_colorbyte(5, 2 * i, DISP_CHAR_INVERT_BIT, i, true);
        // }
        // sleep_ms(2000);
    }

    return 0;
}
