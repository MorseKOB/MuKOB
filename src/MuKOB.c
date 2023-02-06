/**
 * MuKOB main application.
 * 
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 * 
*/
#include <stdio.h>

#include "system_defs.h"
// (some defines in the above are used in the following, so must be included first)
#include "display_ili9341.h"
#include "mukboard.h"

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

    if (option_value(OPTION_DEBUG)) {
        buzzer_beep(500);
    }

    printf("int: %d uint32_t %d, short int: %d uint16_t: %d char: %d uint8_t: %d\n", sizeof(int), sizeof(uint32_t), sizeof(short int), sizeof(uint16_t), sizeof(char), sizeof(uint8_t));

    uint8_t color = 0;
    while(true) {
        color++;
        uint8_t fgc = fg_from_cb(color);
        uint8_t bgc = bg_from_cb(color);
        if (fgc == bgc) {
            continue;
        }
        disp_clear(true);
        sleep_ms(500);
        disp_set_text_colors(fg_from_cb(color), bg_from_cb(color));
        disp_font_test();
        sleep_ms(1000);
        led_on_off(say_hi);

        disp_clear(true);
        margins_set(0, 0, DISP_CHAR_ROWS - 1, DISP_CHAR_COLS - 1);
        char ca = 0;
        for (int i = 0; i < 4 * DISP_CHAR_COLS * DISP_CHAR_ROWS; i++) {
            for (int col = 0; col < DISP_CHAR_COLS; col++) {
                char c = '@' + ca + col;
                c &= 0x1F;
                c |= 0x40;
                printc(c, false);
            }
            disp_paint();
            ca++;
        }
//        kobnet_set_network();
//        kobnet_test();

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
