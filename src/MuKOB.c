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
// #include "display.h"
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

    while(true) {
//        disp_font_test();
//        sleep_ms(1000);

//        kobnet_set_network();
//        kobnet_test();

        // disp_clear(true);
        // char d = '\x01';
        // for (unsigned int r = 0; r < 6; r++) {
        //     for (unsigned int c = 0; c < 14; c++) {
        //         disp_char(r, c, d++, false);
        //     }
        //     disp_row_paint(r);
        // }
        // sleep_ms(1000);
        // // Test setting the invert on chars directly in the text buffer
        // // then paint.
        // for (int i = 0; i < 10; i++) {
        //     int offset = (5 * DISP_CHAR_COLS) + i;
        //     *(text_full_screen + offset) = (*(text_full_screen + offset) | DISP_CHAR_INVERT_BIT);
        // }
        // disp_update(true);
        // sleep_ms(1000);
        // // Test scrolling 3 rows up
        // disp_rows_scroll_up(2, 4, true);
        // sleep_ms(1000);
        // // Test clearing a row
        // disp_row_clear(1, true);
        // sleep_ms(1000);
        // // Test displaying a string
        // disp_string(4, 2, "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS.", false, true);
        // sleep_ms(1000);
        // // Simulate the MuKOB screen
        // disp_clear(true);
        // const char status_bar[] = {0x0B,0x1B,'1','0','8',' ',0x1A,'2','5',' ',0x01,0x04,0x1F,0x1E,0x00};
        // disp_string(0, 0, status_bar, true, false);
        // disp_string(1,0,"HOW EATING OYSTERS COULD HELP PROTECT THE ", false, false);
        // disp_string(5,0,"BBC WORLD NE \x11", false, true);
        led_on_off(say_hi);
        sleep_ms(5000);
    }

    return 0;
}
