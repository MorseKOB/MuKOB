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

#define _MAX_PENDING_ALARMS 16

typedef bool (*get_msg_nowait_fn)(cmt_msg_t* msg);

// microsecond overhead (typical/approx) for handling an alarm and posting a message.
#define ALARM_HANDLER_OVERHEAD_US 300L

typedef struct _alarm_handler_data_ {
    uint8_t corenum;
    cmt_msg_t* client_msg;
    alarm_id_t alarm_id;
    alarm_index_t alarm_index;
    uint32_t ms;
    uint32_t created_ms;
} alarm_handler_data_t;

static alarm_handler_data_t* _pending_alarm_datas[_MAX_PENDING_ALARMS]; // Space for 16 pending timers (this way we don't need to malloc/free)
static int _pending_alarms = ALARM_INDEX_INVALID;

static void _alarm_handler_data_free(alarm_handler_data_t* alarm_handler_data_to_free);

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
int64_t _alarm_handler(alarm_id_t id, void* data) {
    alarm_handler_data_t* handler_data = (alarm_handler_data_t*)data;
    uint8_t corenum = handler_data->corenum;
    cmt_msg_t* client_msg = handler_data->client_msg;
    _alarm_handler_data_free(handler_data);
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

static void _alarm_free_stale() {
    uint32_t now = us_to_ms(time_us_64());
    for (int ai = 0; ai < _MAX_PENDING_ALARMS; ai++) {
        alarm_handler_data_t* alarm_handler_data = _pending_alarm_datas[ai];
        if (alarm_handler_data) {
            // If it is now more than 5 seconds later than the alarm time, cancel and free it.
            if (now > (alarm_handler_data->created_ms + alarm_handler_data->ms + 5000)) {
                // For some reason, this alarm still exists
                if (alarm_handler_data->alarm_id > 0) {
                    cancel_alarm(alarm_handler_data->alarm_id);
                }
                _alarm_handler_data_free(alarm_handler_data);
            }
        }
    }
}

static void _alarm_handler_data_free(alarm_handler_data_t* alarm_handler_data_to_free) {
    alarm_index_t ai = (alarm_handler_data_to_free ? alarm_handler_data_to_free->alarm_index : ALARM_INDEX_INVALID);
    if (ALARM_INDEX_INVALID != ai) {
        alarm_handler_data_t* alarm_handler_data = _pending_alarm_datas[alarm_handler_data_to_free->alarm_index];
        if (alarm_handler_data) {
            free(alarm_handler_data);
            _pending_alarm_datas[ai] = NULL;
            _pending_alarms--;
        }
    }
}

void alarm_cancel(alarm_index_t ai) {
    // Find the alarm data
    if (ALARM_INDEX_INVALID != ai) {
        alarm_handler_data_t* alarm_handler_data = _pending_alarm_datas[ai];
        if (alarm_handler_data) {
            if (alarm_handler_data->alarm_id > 0) {
                cancel_alarm(alarm_handler_data->alarm_id);
            }
            _alarm_handler_data_free(alarm_handler_data);
        }
    }
}

alarm_index_t alarm_set_ms(uint32_t ms, cmt_msg_t* msg) {
    // First, if we don't have any open slots - see if we can free one up
    alarm_index_t ai = ALARM_INDEX_INVALID;
    alarm_id_t alarm_id = -2;
    if (_pending_alarms == _MAX_PENDING_ALARMS) {
        _alarm_free_stale();
    }
    // Find a free slot
    for (int i = 0; i < _MAX_PENDING_ALARMS; i++) {
        if (!_pending_alarm_datas[i]) {
            ai = i;
            break;
        }
    }
    uint64_t delay_us = (ms * 1000) - ALARM_HANDLER_OVERHEAD_US;
    alarm_handler_data_t* handler_data = malloc(sizeof(alarm_handler_data_t));
    handler_data->corenum = (uint8_t)get_core_num();
    handler_data->client_msg = msg;
    if (ALARM_INDEX_INVALID != ai) {
        alarm_id = add_alarm_in_us(delay_us, _alarm_handler, handler_data, false);
    }
    handler_data->alarm_id = alarm_id;
    handler_data->ms = ms;
    handler_data->created_ms = us_to_ms(time_us_64());
    // Store it in the slot
    if (ALARM_INDEX_INVALID != ai) {
        handler_data->alarm_index = ai;
        _pending_alarm_datas[ai] = handler_data;
        _pending_alarms++;
    }
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
        // The alarm will trigger the handler, which will post the client message and free things up.
    }

    return (ai);
}

void cmt_init() {
    memset(_pending_alarm_datas, 0, sizeof(_pending_alarm_datas));
    _pending_alarms = 0;
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
