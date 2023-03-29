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
#include "cmt.h"
#include "core1_main.h"

#define CORE0_QUEUE_ENTRIES_MAX 32
#define CORE1_QUEUE_ENTRIES_MAX 32

queue_t core0_queue;
queue_t core1_queue;

void get_core0_msg_blocking(cmt_msg_t* msg) {
    queue_remove_blocking(&core0_queue, msg);
}

bool get_core0_msg_nowait(cmt_msg_t* msg) {
    return (queue_try_remove(&core0_queue, msg));
}

void get_core1_msg_blocking(cmt_msg_t* msg) {
    queue_remove_blocking(&core1_queue, msg);
}

bool get_core1_msg_nowait(cmt_msg_t* msg) {
    return (queue_try_remove(&core1_queue, msg));
}

void multicore_init() {
    queue_init(&core0_queue, sizeof(cmt_msg_t), CORE0_QUEUE_ENTRIES_MAX);
    queue_init(&core1_queue, sizeof(cmt_msg_t), CORE1_QUEUE_ENTRIES_MAX);
    cmt_init();
}

void post_to_core0_blocking(cmt_msg_t *msg) {
    queue_add_blocking(&core0_queue, msg);
}

bool post_to_core0_nowait(cmt_msg_t *msg) {
    return (queue_try_add(&core0_queue, msg));
}

void post_to_core1_blocking(cmt_msg_t *msg) {
    queue_add_blocking(&core1_queue, msg);
}

bool post_to_core1_nowait(cmt_msg_t *msg) {
    return (queue_try_add(&core1_queue, msg));
}

void post_to_cores_blocking(cmt_msg_t* msg) {
    queue_add_blocking(&core0_queue, msg);
    queue_add_blocking(&core1_queue, msg);
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

