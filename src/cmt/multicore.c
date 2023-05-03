/**
 * MuKOB Multicore common.
 *
 * Containes the data structures and routines to handle multicore functionality.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "multicore.h"
#include "system_defs.h"
#include "cmt.h"
#include "core1_main.h"
#include "mkboard.h"
#include "mkdebug.h"

#include <stdio.h>

#define CORE0_QUEUE_ENTRIES_MAX 32
#define CORE1_QUEUE_ENTRIES_MAX 32

static bool _initialized = false;

queue_t core0_queue;
queue_t core1_queue;

void get_core0_msg_blocking(cmt_msg_t* msg) {
    queue_remove_blocking(&core0_queue, msg);
}

bool get_core0_msg_nowait(cmt_msg_t* msg) {
    register bool retrieved = false;
    uint32_t flags = save_and_disable_interrupts();
    retrieved = queue_try_remove(&core0_queue, msg);
    restore_interrupts(flags);

    return (retrieved);
}

void get_core1_msg_blocking(cmt_msg_t* msg) {
    queue_remove_blocking(&core1_queue, msg);
}

bool get_core1_msg_nowait(cmt_msg_t* msg) {
    register bool retrieved = false;
    uint32_t flags = save_and_disable_interrupts();
    retrieved = queue_try_remove(&core1_queue, msg);
    restore_interrupts(flags);

    return (retrieved);
}

void multicore_module_init() {
    assert(!_initialized);
    _initialized = true;
    queue_init(&core0_queue, sizeof(cmt_msg_t), CORE0_QUEUE_ENTRIES_MAX);
    queue_init(&core1_queue, sizeof(cmt_msg_t), CORE1_QUEUE_ENTRIES_MAX);
    cmt_module_init();
}

static void _check_q0_level(char c, int id) {
    if (mk_debug()) {
        if (CORE0_QUEUE_ENTRIES_MAX - queue_get_level(&core0_queue) < 4) {
            cmt_msg_t msg;
            uint32_t now = now_ms();
            for (int i = 0; queue_get_level(&core0_queue) > 0; i++) {
                get_core0_msg_blocking(&msg);
                printf("\n!!! Q0-%02d:%#04.4x TIQ:%d !!!", i, msg.id, now - msg.t);
            }
            panic("Q0 almost full. P%c:%#04.4x", c, id);
        }
    }
}

static void _check_q1_level(char c, int id) {
    if (mk_debug()) {
        if (CORE1_QUEUE_ENTRIES_MAX - queue_get_level(&core1_queue) < 4) {
            cmt_msg_t msg;
            uint32_t now = now_ms();
            for (int i = 0; queue_get_level(&core1_queue) > 0; i++) {
                get_core1_msg_blocking(&msg);
                printf("\n!!! Q1-%02d:%#04.4x !!!", i, msg.id, now - msg.t);
            }
            panic("Q1 almost full. P%c:%#04.4x TIQ:%d", c, id);
        }
    }
}

void post_to_core0_blocking(cmt_msg_t *msg) {
    msg->t = now_ms();
    _check_q0_level('B', msg->id);
    uint32_t flags = save_and_disable_interrupts();
    queue_add_blocking(&core0_queue, msg);
    restore_interrupts(flags);
}

bool post_to_core0_nowait(cmt_msg_t *msg) {
    msg->t = now_ms();
    _check_q0_level('N', msg->id);
    register bool posted = false;
    uint32_t flags = save_and_disable_interrupts();
    posted = queue_try_add(&core0_queue, msg);
    restore_interrupts(flags);

    return (posted);
}

void post_to_core1_blocking(cmt_msg_t* msg) {
    msg->t = now_ms();
    _check_q1_level('B', msg->id);
    uint32_t flags = save_and_disable_interrupts();
    queue_add_blocking(&core1_queue, msg);
    restore_interrupts(flags);
}

bool post_to_core1_nowait(cmt_msg_t* msg) {
    msg->t = now_ms();
    _check_q1_level('N', msg->id);
    register bool posted = false;
    uint32_t flags = save_and_disable_interrupts();
    posted = queue_try_add(&core1_queue, msg);
    restore_interrupts(flags);

    return (posted);
}

void post_to_cores_blocking(cmt_msg_t* msg) {
    post_to_core0_blocking(msg);
    post_to_core1_blocking(msg);
}

uint16_t post_to_cores_nowait(cmt_msg_t* msg) {
    uint16_t retval = 0;
    if (post_to_core0_nowait(msg)) {
        retval |= 0x01;
    }
    if (post_to_core1_nowait(msg)) {
        retval |= 0x02;
    }
    return (retval);
}

void start_core1() {
    // Start up the Core 1 main.
    //
    // Core 1 must be started before FIFO interrupts are enabled.
    // (core1 launch uses the FIFO's, so enabling interrupts before
    // the FIFO's are used for the launch will result in unexpected behaviour.
    //
    multicore_launch_core1(core1_main);
}

