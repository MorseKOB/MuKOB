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

// TODO: Since this can be used by both cores, a mutex should be used to protect the methods
//       that set/get/clear scheduled messages.

#define _SCHEDULED_MESSAGES_MAX 16
#define _INVALID_ALARM_ID ((alarm_id_t)-2)

typedef bool (*get_msg_nowait_fn)(cmt_msg_t* msg);

// microsecond overhead (typical/approx) for handling an alarm and posting a message.
#define ALARM_HANDLER_OVERHEAD_US (200L)

typedef struct _scheduled_msg_data_ {
    bool in_use;
    uint8_t corenum;
    const cmt_msg_t* client_msg;
    alarm_id_t alarm_id;
    scheduled_msg_id_t sched_msg_id;
    uint32_t ms;
    uint32_t created_ms;
} _scheduled_msg_data_t;

static _scheduled_msg_data_t _scheduled_message_datas[_SCHEDULED_MESSAGES_MAX]; // Pending alarms (no malloc/free)
static int _scheduled_messages = SCHED_MSG_ID_INVALID;

static void _scheduled_msg_clear(int sched_msg_id);

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
    _scheduled_msg_data_t* scheduled_msg_data = (_scheduled_msg_data_t*)data;
    if (scheduled_msg_data) {
        uint8_t corenum = scheduled_msg_data->corenum;
        const cmt_msg_t* client_msg = scheduled_msg_data->client_msg;
        _scheduled_msg_clear(scheduled_msg_data->sched_msg_id);
        if (client_msg) {
            if (corenum == 0) {
                post_to_core0_blocking(client_msg);
            }
            else if (corenum == 1) {
                post_to_core1_blocking(client_msg);
            }
            else {
                error_printf("CMT - `alarm_handler` got unknown core number: %d", (int)corenum);
            }
        }
    }

    return (0L); // Don't reschedule
}

static void _scheduled_msg_clear_stale() {
    uint32_t now = us_to_ms(time_us_64());
    for (int id = 0; id < _SCHEDULED_MESSAGES_MAX; id++) {
        _scheduled_msg_data_t* scheduled_msg_data = &_scheduled_message_datas[id];
        if (scheduled_msg_data->in_use) {
            // If it is now more than 5 seconds later than the alarm time, cancel and free it.
            if (now > (scheduled_msg_data->created_ms + scheduled_msg_data->ms + 5000)) {
                // For some reason, this alarm still exists
                if (scheduled_msg_data->alarm_id > 0) {
                    cancel_alarm(scheduled_msg_data->alarm_id);
                }
                _scheduled_msg_clear(id);
            }
        }
    }
}

static void _scheduled_msg_clear(int id) {
    if (SCHED_MSG_ID_INVALID != id) {
        _scheduled_message_datas[id].in_use = false;
        _scheduled_message_datas[id].alarm_id = _INVALID_ALARM_ID;
        _scheduled_messages--;
    }
}

void scheduled_msg_cancel(scheduled_msg_id_t id) {
    // Find the alarm data
    if (SCHED_MSG_ID_INVALID != id && id >= 0 && id < _SCHEDULED_MESSAGES_MAX) {
        _scheduled_msg_data_t* scheduled_msg_data = &_scheduled_message_datas[id];
        if (scheduled_msg_data->in_use) {
            if (scheduled_msg_data->alarm_id > 0) {
                cancel_alarm(scheduled_msg_data->alarm_id);
            }
            _scheduled_msg_clear(id);
        }
    }
}

scheduled_msg_id_t schedule_msg_in_ms(uint32_t ms, const cmt_msg_t* msg) {
    // First, if we don't have any open slots - see if we can free one up
    scheduled_msg_id_t id = SCHED_MSG_ID_INVALID;
    alarm_id_t alarm_id = -2;
    uint8_t core_num = (uint8_t)get_core_num();
    if (_scheduled_messages == _SCHEDULED_MESSAGES_MAX) {
        _scheduled_msg_clear_stale();
    }
    // Find a free slot
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        if (!_scheduled_message_datas[i].in_use) {
            id = i;
            break;
        }
    }
    if (SCHED_MSG_ID_INVALID != id && ms > 0) {
        _scheduled_msg_data_t* handler_data = &_scheduled_message_datas[id];
        uint64_t delay_us = ((ms * 1000) - ALARM_HANDLER_OVERHEAD_US);
        handler_data->corenum = core_num;
        handler_data->client_msg = msg;
        handler_data->ms = ms;
        handler_data->created_ms = us_to_ms(time_us_64()); // 'now' in millis
        // Store it in the slot
        handler_data->sched_msg_id = id;
        handler_data->in_use = true;
        handler_data->alarm_id = alarm_id; // Store the 'flag' alarm ID
        _scheduled_messages++;
        alarm_id = add_alarm_in_us(delay_us, _alarm_handler, handler_data, false);
        handler_data->alarm_id = alarm_id; // Store the 'real' alarm ID
    }
    if (SCHED_MSG_ID_INVALID == id || alarm_id < 1) {
        char* reason = (SCHED_MSG_ID_INVALID == id ? "No alarm slots available" : "Call to `add_alarm_in_us` failed");
        error_printf("CMT - `alarm_set_ms` could not set alarm due to: %s. Posting message immediately.\n", reason);
        // handle immediately so they get their message
        if (core_num == 0) {
            post_to_core0_blocking(msg);
        }
        else if (core_num == 1) {
            post_to_core1_blocking(msg);
        }
        _scheduled_msg_clear(id);
        id = SCHED_MSG_ID_INVALID;
    }

    return (id);
}

extern scheduled_msg_id_t scheduled_message_get(const cmt_msg_t* msg) {
    scheduled_msg_id_t id = SCHED_MSG_ID_INVALID;
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        if (_scheduled_message_datas[i].in_use && msg == _scheduled_message_datas[i].client_msg) {
            id = i;
            break;
        }
    }
    return (id);
}

void cmt_init() {
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_message_datas[i].alarm_id = 0;
        _scheduled_message_datas[i].sched_msg_id = i;
        _scheduled_message_datas[i].client_msg = NULL;
        _scheduled_message_datas[i].in_use = false;
    }
    _scheduled_messages = 0;
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
