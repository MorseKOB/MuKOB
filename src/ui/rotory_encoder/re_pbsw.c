/**
 * @brief Push button switch on the rotory encoder.
 * @ingroup ui
 *
 * This provides input from the momentary action push-button switch on the rotory encoder.
 *
 * Copyright 2023 AESilky
 *
 * SPDX-License-Identifier: MIT
 */
#include "system_defs.h"
#include "re_pbsw.h"

#include "cmt.h"

#include <stdio.h>

// static char event_str[128];

static void _gpio_event_string(char* buf, uint32_t events);

void re_pbsw_irq_handler(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        cmt_msg_t msg = { MSG_WIRE_CONNECT_TOGGLE };
        postBEMsgBlocking(&msg);
    }
    if (events & GPIO_IRQ_EDGE_RISE) {
        //printf("re switch released\n");
    }
}

void re_pbsw_module_init() {
    // GPIO is initialized in `mkboard.c` with the rest of the board.
}
