/**
 * MuKOB Core 1 main start-up and management.
 *
 * This containes the main routine (the entry point) for operations on Core 1.
 *
 * See the core1_main.h header for important information.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "core1_main.h"

#include "cmt.h"
#include "mkboard.h"
#include "multicore.h"
#include "ui.h"
#include "pico/multicore.h"

void core1_main() {
    info_printf("CORE-%d - *** Started ***\n", get_core_num());

    // Set up the UI
    ui_init();

    // Enter into the (endless) UI Message Dispatching Loop
    message_loop(&ui_msg_loop_cntx);
}
