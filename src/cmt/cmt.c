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
#include "system_defs.h"
#include "mkboard.h"
#include "mkdebug.h"
#include "util.h"
#include "hardware/structs/nvic.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdlib.h>
#include <string.h>


#define _SCHEDULED_MESSAGES_MAX 16
#define _SM_OVERHEAD_US_PER_MS 35 // From testing (@SEE be._handle_be_test)
#define _SMD_FREE_INDICATOR (-1)

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

static bool _msg_loop_0_running = false;
static bool _msg_loop_1_running = false;

static proc_status_accum_t _psa[2]; // One Proc Status Accumulator for each core
static proc_status_accum_t _psa_sec[2]; // Proc Status Accumulator per second for each core

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
    bool success = add_repeating_timer_us((1000 - _SM_OVERHEAD_US_PER_MS), _schd_msg_timer_callback, NULL, &_schd_msg_timer_data);
    if (!success) {
        error_printf(false, "CMT - Could not create repeating timer for scheduled messages.\n");
        panic("CMT - Could not create repeating timer for scheduled messages.");
    }
}

bool cmt_message_loop_0_running() {
    return (_msg_loop_0_running);
}

bool cmt_message_loop_1_running() {
    return (_msg_loop_1_running);
}

bool cmt_message_loops_running() {
    return (_msg_loop_0_running && _msg_loop_1_running);
}

void cmt_handle_sleep(cmt_msg_t* msg) {
    cmt_sleep_fn fn = msg->data.cmt_sleep->sleep_fn;
    if (fn) {
        (fn)(msg->data.cmt_sleep->user_data);
    }
}

void cmt_proc_status_sec(proc_status_accum_t* psas, uint8_t corenum) {
    if (corenum < 2) {
        proc_status_accum_t* psa_sec = &_psa_sec[corenum];
        proc_status_accum_t psa;
        int64_t cs = 0;
        do {
            psa.cs = psa_sec->cs;   // Get the 'per-sec' checksum
            psa.core_temp = psa_sec->core_temp;
            psa.idle = psa_sec->idle;
            cs = psa.idle;
            psa.retrived = psa_sec->retrived;
            cs += psa.retrived;
            psa.t_active = psa_sec->t_active;
            cs += psa.t_active;
            psa.t_idle = psa_sec->t_idle;
            cs += psa.t_idle;
            psa.t_msgr = psa_sec->t_msgr;
            cs += psa.t_msgr;
            psa.int_status = psa_sec->int_status;
            cs += psa.int_status;
            psa.ts_psa = psa_sec->ts_psa;

        } while(psa.cs != cs);
        psas->core_temp = psa.core_temp;
        psas->idle = psa.idle;
        psas->retrived = psa.retrived;
        psas->t_active = psa.t_active;
        psas->t_idle = psa.t_idle;
        psas->t_msgr = psa.t_msgr;
        psas->int_status = psa.int_status;
        psas->ts_psa = psa.ts_psa;
        psas->cs = psa.cs;
    }
}

int cmt_sched_msg_waiting() {
    int count = 0;
    uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (_SMD_FREE_INDICATOR != smd->remaining) {
            count++;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);

    return (count);
}

void cmt_sleep_ms(int32_t ms, cmt_sleep_fn sleep_fn, void* user_data) {
    bool scheduled = false;

    uint8_t core_num = (uint8_t)get_core_num();
    uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    // Get a free smd
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (_SMD_FREE_INDICATOR == smd->remaining) {
            // This is free;
            smd->sleep_msg.id = MSG_CMT_SLEEP;
            smd->sleep_msg.data.cmt_sleep->sleep_fn = sleep_fn;
            smd->sleep_msg.data.cmt_sleep->user_data = user_data;
            smd->client_msg = &smd->sleep_msg;
            smd->ms_requested = ms;
            smd->corenum = core_num;
            smd->remaining = ms;
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

    uint8_t core_num = (uint8_t)get_core_num();
    uint32_t flags = save_and_disable_interrupts();
    mutex_enter_blocking(&sm_mutex);
    // Get a free smd
    for (int i = 0; i < _SCHEDULED_MESSAGES_MAX; i++) {
        _scheduled_msg_data_t* smd = &_scheduled_message_datas[i];
        if (_SMD_FREE_INDICATOR == smd->remaining) {
            // This is free;
            smd->client_msg = msg;
            smd->ms_requested = ms;
            smd->corenum = core_num;
            smd->remaining = ms;
            scheduled = true;
            break;
        }
    }
    mutex_exit(&sm_mutex);
    restore_interrupts(flags);
    if (!scheduled) {
        panic("CMT - No SM Data slot available for use.");
    }
}


void scheduled_msg_cancel(msg_id_t sched_msg_id) {
    uint32_t flags = save_and_disable_interrupts();
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
    uint32_t flags = save_and_disable_interrupts();
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

/*
 * Endless loop reading and dispatching messages.
 * This is called/started once from each core, so two instances are running.
 */
void message_loop(const msg_loop_cntx_t* loop_context) {
    // Setup occurs once when called by a core.
    uint8_t corenum = loop_context->corenum;
    get_msg_nowait_fn get_msg_function = (corenum == 0 ? get_core0_msg_nowait : get_core1_msg_nowait);
    cmt_msg_t msg;
    const idle_fn* idle_functions = loop_context->idle_functions;
    proc_status_accum_t *psa = &_psa[corenum];
    proc_status_accum_t *psa_sec = &_psa_sec[corenum];
    psa->ts_psa = now_ms();

    // Indicate that the message loop is running for the calling core.
    if (corenum == 0) {
        _msg_loop_0_running = true;
    }
    else {
        _msg_loop_1_running = true;
    }

    // Enter into the endless loop reading and dispatching messages to the handlers...
    while (1) {
        uint32_t t_start = now_ms();
        // Store and reset the process status accumulators once every second
        if (t_start - psa->ts_psa >= ONE_SECOND_MS) {
            int64_t cs = 0;
            psa_sec->cs = -1;
            psa_sec->idle = psa->idle;
            cs += psa_sec->idle;
            psa->idle = 0;
            psa_sec->retrived = psa->retrived;
            cs += psa_sec->retrived;
            psa->retrived = 0;
            psa_sec->t_active = psa->t_active;
            cs += psa_sec->t_active;
            psa->t_active = 0;
            psa_sec->t_idle = psa->t_idle;
            cs += psa_sec->t_idle;
            psa->t_idle = 0;
            psa_sec->t_msgr = psa->t_msgr;
            cs += psa_sec->t_msgr;
            psa->t_msgr = 0;
            psa_sec->int_status = nvic_hw->iser;
            cs += psa_sec->int_status;
            psa_sec->core_temp = onboard_temp_c();
            psa_sec->ts_psa = t_start;
            psa->ts_psa = t_start;
            psa_sec->cs = cs;
        }

        if (get_msg_function(&msg)) {
            uint32_t as = now_ms();
            psa->t_msgr += as - t_start;
            psa->retrived++;
            // Find the handler
            const msg_handler_entry_t** handler_entries = loop_context->handler_entries;
            while (*handler_entries) {
                const msg_handler_entry_t* handler_entry = *handler_entries++;
                if (msg.id == handler_entry->msg_id) {
                    handler_entry->msg_handler(&msg);
                }
            }
            // No more handlers found for this message.
            uint32_t ht = now_ms() - as;
            psa->t_active += ht;
        }
        else {
            // No message available, allow next idle function to run
            int32_t is = now_ms();
            psa->t_msgr += is - t_start;
            psa->idle++;
            const idle_fn idle_function = *idle_functions;
            if (idle_function) {
                idle_function();
                idle_functions++; // Next time do the next one...
            }
            else {
                // end of function list
                idle_functions = loop_context->idle_functions; // reset the pointer
            }
            uint32_t it = now_ms() - is;
            psa->t_idle += it;
        }
    }
}

void cmt_module_init() {
    mutex_enter_blocking(&sm_mutex);
    _scheduled_msg_init();
    mutex_exit(&sm_mutex);
}
