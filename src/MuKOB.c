/**
 * MuKOB main application.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "pico/binary_info.h"
//
#include "hardware/irq.h"
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/datetime.h"

#include "system_defs.h" // Main system/board/application definitions
//
#include "core1_main.h"
#include "be.h"
#include "mkboard.h"
#include "multicore.h"

#define DOT_MS 60 // Dot at 20 WPM
#define UP_MS  DOT_MS
#define DASH_MS (2 * DOT_MS)
#define CHR_SP (3 * DOT_MS)

 // 'H' (....) 'I' (..)
static int32_t say_hi[] = {
    DOT_MS,
    UP_MS,
    DOT_MS,
    UP_MS,
    DOT_MS,
    UP_MS,
    DOT_MS,
    CHR_SP,
    DOT_MS,
    UP_MS,
    DOT_MS,
    1000, // Pause before repeating
    0 };

int main()
{
    // useful information for picotool
    bi_decl_if_func_used(bi_2pins_with_func(PICO_DEFAULT_UART_RX_PIN, PICO_DEFAULT_UART_TX_PIN,
        GPIO_FUNC_UART));
    bi_decl(bi_program_description("Micro version of MorseKOB, with built-in display and terminal UI"));

    board_init();


    // We never expect to end
    do {
        if (option_value(OPTION_DEBUG)) {
            buzzer_beep(250);
        }
        led_on_off(say_hi);

        // Initialize our multi-core system
        multicore_init();

        // Launch Core 1 - The UI
        start_core1();

        // Set up the Backend
        be_init();

        // Enter into the (endless) Backend Message Dispatching Loop
        enter_message_loop(&be_msg_loop_cntx);
    } while (1);

    // How did we get here?!
    error_printf("MuKOB - Somehow we are out of our endless loop!!!");
    return 0;
}
