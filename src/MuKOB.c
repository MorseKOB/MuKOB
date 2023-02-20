/**
 * MuKOB main application.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
//#include <stdio.h>
//#include <stdarg.h>

#include "system_defs.h"
// (some defines in the above are used in the following, so must be included first)
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/datetime.h"
#include "net.h"
#include "mukboard.h"
#include "display.h"
#include "display_ili9341.h"
#include "font.h"
#include "mkwire.h"

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

    if (option_value(OPTION_DEBUG)) {
        buzzer_beep(250);
    }

    // Try to connect to the MKOB Server wire 108
    // mkwire_init("192.168.68.85", MKOBSERVER_PORT_DEFAULT, "ES, Ed, WA (MuKOB)");
    // mkwire_connect(90); // Test wire
    // sleep_ms(15000);

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
        disp_printf(true, "`disp_printf(%b, \"%s\")`\n", true, "some string");
        disp_font_test();
        //led_on_off(say_hi);

        // ili9341_colors_show();
        // sleep_ms(2000);

        disp_c16_color_chart();
        sleep_ms(2000);

        disp_clear(true);
        margins_set(0, 0, DISP_CHAR_LINES - 1, DISP_CHAR_COLS - 1);
        char ca = 0;
        for (int i = 0; i < 22; i++) {
            for (int col = 0; col < DISP_CHAR_COLS; col++) {
                char c = '@' + ca + col;
                c &= 0x1F;
                c |= 0x40;
                printc(c, false);
            }
            disp_paint();
            ca++;
        }
        sleep_ms(2000);

        // Leave the text on the screen and test ili9341 scroll
        disp_string(0, 0,  "\024\025W:108 S:25 \022\023 \016 \002 \012\013\014\015", true, true);
        disp_string(1, 0,  "                        ", true, true);
        disp_string(12, 0, "098765432109876543210987", false, true);
        disp_string(17, 0, "A 1 B 2 C 3 D 4 E 5 F 6 ", false, true);
        disp_string(18, 0, " A 1 B 2 C 3 D 4 E 5 F 6", false, true);
        disp_string(DISP_CHAR_LINES - 1, 0, " KOB      10:31      AES", true, true);
        disp_char(DISP_CHAR_LINES - 1, 0, '\200', true);
        ili9341_scroll_set_area(2 * FONT_HEIGHT, 3 * FONT_HEIGHT);
        for (unsigned char i = 0; i < 64; i++) {
            sleep_ms(20);
            ili9341_scroll_set_start((2 * FONT_HEIGHT) + (i * FONT_HEIGHT) % (ILI9341_HEIGHT - (5 * FONT_HEIGHT)));
            disp_char((i % DISP_CHAR_LINES), 0, i, true);
        }
        sleep_ms(3000);
        ili9341_scroll_exit();
    }

    return 0;
}
