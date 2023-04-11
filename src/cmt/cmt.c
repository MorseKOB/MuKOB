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
#define _OVERHEAD_US_PER_MS 30 // From testing

typedef bool (*get_msg_nowait_fn)(cmt_msg_t* msg);

typedef struct _scheduled_msg_data_ {
    bool in_use;
    struct _scheduled_msg_data_* next;
    uint32_t remaining;
    uint8_t corenum;
    const cmt_msg_t* client_msg;
    scheduled_msg_id_t sched_msg_id;
    uint32_t ms_requested;
    uint32_t created_ms;
} _scheduled_msg_data_t;

static void _scheduled_msg_free(_scheduled_msg_data_t* smd);

auto_init_recursive_mutex(sm_mutex);
static _scheduled_msg_data_t _scheduled_message_datas[_SCHEDULED_MESSAGES_MAX]; // Objects to use (no malloc/free)
static volatile int _scheduled_msg_count = 0;
static _scheduled_msg_data_t* _scheduled_msg_head = NULL;
static _scheduled_msg_data_t* _scheduled_msg_free_list = NULL; // Set in `_scheduled_msg_init`
static repeating_timer_t _schd_msg_timer_data;

/**
 * @brief Repeating alarm callback handler.
 * Handles the from the repeating timer, adjusts the time left and posts a message to
 * the appropriate core when time hits 0.
 *
 * @see repeating_timer_callback_t
 *
 * \param rt repeating time structure containing information about the repeating time. user_data is of primary important to the user
 * \return true to continue repeating, false to stop.
 */
bool _schd_msg_timer_callback(repeating_timer_t* rt) {
    recursive_mutex_enter_blocking(&sm_mutex);
    // Get the head smd
    if (_scheduled_msg_head) {
        _scheduled_msg_head->remaining--; // Subtract a second
        while (_scheduled_msg_head->remaining <= 0) { // Test for less-than or equal just in case
            // This one is done. Free it, post its message and move on to the next.
            _scheduled_msg_data_t* smd = _scheduled_msg_head;
            const cmt_msg_t* client_msg = smd->client_msg;
            uint8_t corenum = smd->corenum;
            _scheduled_msg_head = smd->next;
            _scheduled_msg_free(smd);
            if (client_msg) {
                if (corenum == 0) {
                    post_to_core0_blocking(client_msg);
                }
                else if (corenum == 1) {
                    post_to_core1_blocking(client_msg);
                }
                else {
                    error_printf(false, "CMT - Scheduled Message alarm handler got unknown core number: %d Message: %d", (int)corenum, (int)client_msg->id);
                }
            }
        }
        if (0 == _scheduled_msg_count) {
            _scheduled_msg_head = NULL; // Safeguard
        }
    }
    recursive_mutex_exit(&sm_mutex);
    return (true); // Repeat forever...
}

static inline int _id_from_user(int uid) {
    return (SCHED_MSG_ID_INVALID != uid ? uid - 1 : SCHED_MSG_ID_INVALID);
}
static inline int _id_to_user(int id) {
    return (SCHED_MSG_ID_INVALID != id ? id + 1 : SCHED_MSG_ID_INVALID);
}

void _scheduled_msg_free(_scheduled_msg_data_t* smd) {
    // Put this smd back in the free list
    smd->in_use = false;
    smd->corenum = -1;
    smd->client_msg = NULL;
    smd->next = _scheduled_msg_free_list;
    _scheduled_msg_free_list = smd;
    _scheduled_msg_count--;
    if (_scheduled_msg_count < 0) {
        _scheduled_msg_count = 0; // Safeguard
    }
}

static void _scheduled_msg_init() {
    _scheduled_msg_head = NULL;
    _scheduled_msg_free_list = NULL;
    _scheduled_msg_data_t* smd = NULL;
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        // Initialize these as 'free' and chain them together
        smd = &_scheduled_message_datas[i];
        smd->sched_msg_id = i;
        _scheduled_msg_free(smd);
    }
    _scheduled_msg_count = 0;

    bool success = add_repeating_timer_us(1000, _schd_msg_timer_callback, NULL, &_schd_msg_timer_data);
    if (!success) {
        error_printf(false, "CMT - Could not create repeating timer for scheduled messages.\n");
    }
}

void scheduled_msg_cancel(scheduled_msg_id_t id) {
    id = _id_from_user(id);
    if (SCHED_MSG_ID_INVALID != id && id >= 0 && id < _SCHEDULED_MESSAGES_MAX) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[id];
        recursive_mutex_enter_blocking(&sm_mutex);
        if (smd->in_use) {
            // See if it happens to be the head smd
            if (_scheduled_msg_head == smd) {
                // Yes... pull the 'next' up to the head
                _scheduled_msg_head = smd->next;
                // Adjust the remaining time on the next.
                if (_scheduled_msg_head) {
                    _scheduled_msg_head->remaining += smd->remaining;
                }
            }
            else {
                // Find the smd pointing to it
                _scheduled_msg_data_t* prev = _scheduled_msg_head;
                while (prev) {
                    if (prev->next == smd) {
                        break;
                    }
                    prev = prev->next;
                }
                if (prev) {
                    prev->next = smd->next;
                    if (prev->next) {
                        prev->next->remaining += smd->remaining; // Add the cancelled time to the next.
                    }
                }
            }
            _scheduled_msg_free(smd);
        }
        recursive_mutex_exit(&sm_mutex);
    }
}

scheduled_msg_id_t schedule_msg_in_ms(uint32_t ms, const cmt_msg_t* msg) {
    uint8_t core_num = (uint8_t)get_core_num();
    _scheduled_msg_data_t* smd = NULL;

    // Calculate our overhead and adjust if possible
    uint64_t req_us = ms * 1000;
    uint64_t overhead_us = req_us / _OVERHEAD_US_PER_MS;
    uint32_t overhead_ms = overhead_us / 1000;
    ms -= overhead_ms;

    if (ms > 0) {
        recursive_mutex_enter_blocking(&sm_mutex);
        // Get a free smd
        smd = _scheduled_msg_free_list;

        if (smd) {
            // Take it
            _scheduled_msg_count++;
            _scheduled_msg_free_list = smd->next;

            // Initialize it
            smd->next = NULL;
            smd->in_use = true;
            smd->corenum = core_num;
            smd->client_msg = msg;
            smd->remaining = ms;
            smd->ms_requested = ms;
            smd->created_ms = now_ms();

            // Insert it into the chain
            if (!_scheduled_msg_head) {
                // Nothing in the queue. Make it the head.
                _scheduled_msg_head = smd;
            }
            else {
                _scheduled_msg_data_t* prev = _scheduled_msg_head;
                _scheduled_msg_data_t** p_pnext = &_scheduled_msg_head;
                while (prev) {
                    if (smd->remaining < prev->remaining) {
                        // This is where it should be inserted
                        break;
                    }
                    smd->remaining -= prev->remaining; // Adjust remaining and move to the next
                    p_pnext = &prev->next;
                    prev = prev->next;
                }
                // `prev` is the spot to insert the new smd before, unless it is null.
                if (prev) {
                    smd->next = prev;
                    prev->remaining -= smd->remaining;
                }
                *p_pnext = smd; // Store smd as the previous pointer's 'next'
            }
        }
        recursive_mutex_exit(&sm_mutex);
    }
    if (!smd) {
        char* reason = (ms <= 0 ? "Requested delay <= 0" : "No scheduled message slots available");
        error_printf(false, "CMT - `schedule_msg_in_secs` did not schedule: %s. Posting message immediately.\n", reason);
        // handle immediately so they get their message
        if (core_num == 0) {
            post_to_core0_blocking(msg);
        }
        else if (core_num == 1) {
            post_to_core1_blocking(msg);
        }
    }

    return (smd ? _id_to_user(smd->sched_msg_id) : SCHED_MSG_ID_INVALID);
}

extern scheduled_msg_id_t scheduled_message_get(const cmt_msg_t* msg) {
    recursive_mutex_enter_blocking(&sm_mutex);
    scheduled_msg_id_t id = SCHED_MSG_ID_INVALID;
    _scheduled_msg_data_t* smd = _scheduled_msg_head;
    while (smd) {
        if (smd->in_use && msg == smd->client_msg) {
            id = smd->sched_msg_id;
            break;
        }
        smd = smd->next;
    }
    recursive_mutex_exit(&sm_mutex);
    return (_id_to_user(id));
}

scheduled_msg_id_t scheduled_message_get_by_id(msg_id_t msg) {
    recursive_mutex_enter_blocking(&sm_mutex);
    scheduled_msg_id_t id = SCHED_MSG_ID_INVALID;
    _scheduled_msg_data_t* smd = _scheduled_msg_head;
    while (smd) {
        if (smd->in_use && msg == smd->client_msg->id) {
            id = smd->sched_msg_id;
            break;
        }
        smd = smd->next;
    }
    recursive_mutex_exit(&sm_mutex);
    return (_id_to_user(id));
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
    recursive_mutex_enter_blocking(&sm_mutex);
    _scheduled_msg_init();
    recursive_mutex_exit(&sm_mutex);
}
