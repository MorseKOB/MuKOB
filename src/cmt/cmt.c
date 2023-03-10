/**
 * MuKOB Cooperative Multi-Tasking.
 *
 * Containes message loop, timer, and other CMT enablement functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "cmt.h"
#include "mkboard.h"
#include "util.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdlib.h>
#include <string.h>

typedef bool (*get_msg_nowait_fn)(cmt_msg_t* msg);

// microsecond overhead (typical/approx) for handling an alarm and posting a message.
#define ALARM_HANDLER_OVERHEAD_US 300L

typedef struct _alarm_handler_data_ {
    uint8_t corenum;
    cmt_msg_t* client_msg;
} alarm_handler_data_t;

/**
 * @brief Alarm callback handler.
 * Handles the end of a add_alarm_in_us or add_alarm_in_ms call and posts a message to
 * the appropriate core.
 *
 * @see alarm_callback_t
 *
 * @return <0 to reschedule the same alarm this many us from the time the alarm was previously scheduled to fire
 * @return >0 to reschedule the same alarm this many us from the time this method returns
 * @return 0 to not reschedule the alarm
 */
static int64_t _alarm_handler(alarm_id_t id, void* data) {
    alarm_handler_data_t* handler_data = (alarm_handler_data_t*)data;
    uint8_t corenum = handler_data->corenum;
    cmt_msg_t* client_msg = handler_data->client_msg;
    free(handler_data);
    if (corenum == 0) {
        post_to_core0_blocking(client_msg);
    }
    else if (corenum == 1) {
        post_to_core1_blocking(client_msg);
    }
    else {
        error_printf("CMT - `alarm_handler` got unknown core number: %d", (int)corenum);
    }

    return (0L); // Don't reschedule
}

void alarm_set_ms(uint32_t ms, cmt_msg_t* msg) {
    uint64_t delay_us = (ms * 1000) - ALARM_HANDLER_OVERHEAD_US;
    alarm_handler_data_t* handler_data = malloc(sizeof(alarm_handler_data_t));
    handler_data->corenum = (uint8_t)get_core_num();
    handler_data->client_msg = msg;
    int alarm_id = add_alarm_in_us(delay_us, _alarm_handler, handler_data, false);
    if (alarm_id < 0) {
        error_printf("CMT - `alarm_set_ms` could not set alarm.");
        // handle immediately so they get their message
        _alarm_handler(alarm_id, handler_data);
    }
    else if (alarm_id == 0) {
        // The time has already passed, handle immediately.
        _alarm_handler(alarm_id, handler_data);
    }
    else {
        // The alarm will trigger the handler, which will post the client message.
    }
}

void enter_message_loop(msg_loop_cntx_t* loop_context) {
    get_msg_nowait_fn get_msg_function = (loop_context->corenum == 0 ? get_core0_msg_nowait : get_core1_msg_nowait);
    cmt_msg_t msg;
    idle_fn* idle_functions = loop_context->idle_functions;
    while (1) { // Endless loop reading and dispatching messages to the handlers...
        if (get_msg_function(&msg)) {
            // Find the handler
            msg_handler_entry_t** handler_entries = loop_context->handler_entries;
            while (*handler_entries) {
                msg_handler_entry_t* handler_entry = *handler_entries++;
                if (msg.id == handler_entry->msg_id) {
                    handler_entry->msg_handler(&msg);
                }
            }
            // No more handlers found for this message.
            (void)0; // something to set a breakpoint on if wanted
        }
        else {
            // No message available, allow next idle function to run
            idle_fn idle_function = *idle_functions;
            if (idle_function) {
                idle_function(loop_context->idle_data);
                idle_functions++; // Next time do the next one...
            }
            else {
                // end of function list
                idle_functions = loop_context->idle_functions; // reset the pointer
            }
        }
    }
}
