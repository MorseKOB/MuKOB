/**
 * MuKOB Cooperative Multi-Tasking.
 *
 * Containes message loop, scheduled message, and other CMT enablement functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "cmt.h"
#include "mkboard.h"
#include "util.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdlib.h>
#include <string.h>

#define _SCHEDULED_MESSAGES_MAX 16
#define _SMD_FREE_INDICATOR (-1)
#define _OVERHEAD_US_PER_MS 25 // From testing

typedef bool (*get_msg_nowait_fn)(cmt_msg_t* msg);

typedef struct _scheduled_msg_data_ {
    int32_t remaining;
    uint8_t corenum;
    int32_t ms_requested;
    cmt_msg_t* client_msg;
    cmt_msg_t sleep_msg;
} _scheduled_msg_data_t;


auto_init_mutex(sm_mutex);
static _scheduled_msg_data_t _scheduled_message_datas[_SCHEDULED_MESSAGES_MAX]; // Objects to use (no malloc/free)
static repeating_timer_t _schd_msg_timer_data;

/**
 * @brief Repeating alarm callback handler.
 * Handles the from the repeating timer, adjusts the time left and posts a message to
 * the appropriate core when time hits 0.
 *
 * @see repeating_timer_callback_t
 *
 * \param rt repeating time structure containing information about the repeating time. (not used)
 * \return true to continue repeating, false to stop.
 */
bool _schd_msg_timer_callback(repeating_timer_t* rt) {
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (smd->remaining > 0) {
            if (0 == --smd->remaining) {
                if (0 == smd->corenum) {
                    post_to_core0_blocking(smd->client_msg);
                }
                else {
                    post_to_core1_blocking(smd->client_msg);
                }
                smd->remaining = _SMD_FREE_INDICATOR;
            }
        }
    }

    return (true); // Repeat forever
}

static void _scheduled_msg_init() {
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        // Initialize these as 'free'
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        smd->remaining = _SMD_FREE_INDICATOR;
    }

    bool success = add_repeating_timer_us(1000, _schd_msg_timer_callback, NULL, &_schd_msg_timer_data);
    if (!success) {
        error_printf(false, "CMT - Could not create repeating timer for scheduled messages.\n");
    }
}

void cmt_handle_sleep(cmt_msg_t* msg) {
    cmt_sleep_fn* fn = msg->data.sleep_fn;
    if (fn) {
        (*fn)();
    }
}

void cmt_sleep_ms(int32_t ms, cmt_sleep_fn* sleep_fn) {
    bool scheduled = false;

    // Calculate our overhead and adjust if possible
    uint64_t req_us = ms * 1000;
    uint64_t overhead_us = ms * _OVERHEAD_US_PER_MS;
    int32_t adj_ms = (req_us - overhead_us) / 1000;
    adj_ms = (adj_ms > 0 ? adj_ms : 1);

    uint8_t core_num = (uint8_t)get_core_num();
    register uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    // Get a free smd
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (_SMD_FREE_INDICATOR == smd->remaining) {
            // This is free;
            smd->sleep_msg.id = MSG_CMT_SLEEP;
            smd->sleep_msg.data.sleep_fn = sleep_fn;
            smd->client_msg = &smd->sleep_msg;
            smd->ms_requested = ms;
            smd->corenum = core_num;
            smd->remaining = adj_ms;
            scheduled = true;
            break;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);
    if (!scheduled) {
        panic("CMT - No SMD available for use.");
    }
}

void schedule_msg_in_ms(int32_t ms, cmt_msg_t* msg) {
    bool scheduled = false;

    // Calculate our overhead and adjust if possible
    uint64_t req_us = ms * 1000;
    uint64_t overhead_us = ms * _OVERHEAD_US_PER_MS;
    int32_t adj_ms = (req_us - overhead_us) / 1000;
    adj_ms = (adj_ms > 0 ? adj_ms : 1);

    uint8_t core_num = (uint8_t)get_core_num();
    register uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    // Get a free smd
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (_SMD_FREE_INDICATOR == smd->remaining) {
            // This is free;
            smd->client_msg = msg;
            smd->ms_requested = ms;
            smd->corenum = core_num;
            smd->remaining = adj_ms;
            scheduled = true;
            break;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);
    if (!scheduled) {
        panic("CMT - No SMD available for use.");
    }
}


void scheduled_msg_cancel(msg_id_t sched_msg_id) {
    register uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (smd->client_msg && smd->client_msg->id == sched_msg_id) {
            // This matches, so set the remaining to -1;
            smd->remaining = _SMD_FREE_INDICATOR;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);
}

extern bool scheduled_message_exists(msg_id_t sched_msg_id) {
    bool exists = false;
    register uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (smd->client_msg && smd->client_msg->id == sched_msg_id) {
            // This matches
            exists = true;
            break;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);
    return (exists);
}

void message_loop(const msg_loop_cntx_t* loop_context) {
    get_msg_nowait_fn get_msg_function = (loop_context->corenum == 0 ? get_core0_msg_nowait : get_core1_msg_nowait);
    cmt_msg_t msg;
    const idle_fn* idle_functions = loop_context->idle_functions;
    while (1) { // Endless loop reading and dispatching messages to the handlers...
        if (get_msg_function(&msg)) {
            // Find the handler
            const msg_handler_entry_t** handler_entries = loop_context->handler_entries;
            while (*handler_entries) {
                const msg_handler_entry_t* handler_entry = *handler_entries++;
                if (msg.id == handler_entry->msg_id) {
                    handler_entry->msg_handler(&msg);
                }
            }
            // No more handlers found for this message.
            (void)0; // something to set a breakpoint on if wanted
        }
        else {
            // No message available, allow next idle function to run
            const idle_fn idle_function = *idle_functions;
            if (idle_function) {
                idle_function();
                idle_functions++; // Next time do the next one...
            }
            else {
                // end of function list
                idle_functions = loop_context->idle_functions; // reset the pointer
            }
        }
    }
}

void cmt_module_init() {
    mutex_enter_blocking(&sm_mutex);
    _scheduled_msg_init();
    mutex_exit(&sm_mutex);
}
