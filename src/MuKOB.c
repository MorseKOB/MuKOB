/**
 * MuKOB main application.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "pico/binary_info.h"
//
#include "system_defs.h" // Main system/board/application definitions
//
#include "be.h"
#include "mkboard.h"
#include "mkdebug.h"
#include "morse.h"
#include "ui.h"

#define DOT_MS 60 // Dot at 20 WPM
#define UP_MS  DOT_MS
#define DASH_MS (2 * DOT_MS)
#define CHR_SP (3 * DOT_MS)

 // 'H' (....) 'I' (..)
static const int32_t say_hi[] = {
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

//static int32_t qbf[] = { -32767, 2, -460, 180, -230, 60, -60, 60, -60, 60, -60, 60, -230, 60, -460, 60, -60, 60, -60, 180, 60, 60, -230, 60, -60, 60, -60, 180 };

int main()
{
    // useful information for picotool
    bi_decl_if_func_used(bi_2pins_with_func(PICO_DEFAULT_UART_RX_PIN, PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART));
    bi_decl(bi_program_description("Micro version of MorseKOB, with built-in display and terminal UI"));

    // Board/base level initialization
    board_init();

    // Indicate that we are awake
    if (mk_debug()) {
        tone_sound_pattern(150);
    }
    led_on_off(say_hi);

    // Set up the Backend (needs to be done before starting the UI)
    be_module_init();

    // Launch the UI (core-1 Message Dispatching Loop)
    start_ui();

    // Launch the Backend (core-0 (endless) Message Dispatching Loop - never returns)
    start_be();

    // How did we get here?!
    error_printf(true, "MuKOB - Somehow we are out of our endless message loop in `main()`!!!");
    // ZZZ Reboot!!!
    return 0;
}
