/**
 * MuKOB Multicore common.
 *
 * Containes the data structures and routines to handle multicore functionality.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _MK_MULTICORE_H_
#define _MK_MULTICORE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "cmt.h"

/**
 * @file multicore.h
 * @defgroup mk_multicore mk_multicore
 * Common multicore structures, data, and functionality.
 *
 * NOTE: Fifo use
 * MuKOB uses the Fifo for base (core (ha!)) level communication between
 * the cores for the supervisor/system functionality. The Pico SDK warns about
 * using the Fifo, as it is used by the Pico runtime to support multicore operation.
 *
 * MuKOB acknowledges the warnings, and will take care to assure that the Fifo and
 * the MuKOB code_seq is in an appropriate state when operations are performed that will
 * cause the Pico SDK/runtime to use them.
 *
 * For general purpose application communication between the functionality running on
 * the two cores queues will be used.
 *
 * @addtogroup mk_multicore
 * @include multicore.c
 *
*/

/**
 * @brief Get a message for Core 0 (from the Core 0 queue). Block until a message can be read.
 *
 * @param msg Pointer to a buffer for the message.
 */
void get_core0_msg_blocking(cmt_msg_t *msg);

/**
 * @brief Get a message for Core 0 (from the Core 0 queue) if available, but don't wait for one.
 *
 * @param msg Pointer to a buffer for the message.
 * @return true If a message was retrieved.
 * @return false If no message was available.
 */
bool get_core0_msg_nowait(cmt_msg_t* msg);

/**
 * @brief Get a message for Core 1 (from the Core 1 queue). Block until a message can be read.
 *
 * @param msg Pointer to a buffer for the message.
 */
void get_core1_msg_blocking(cmt_msg_t* msg);

/**
 * @brief Get a message for Core 1 (from the Core 1 queue) if available, but don't wait.
 *
 * @param msg Pointer to a buffer for the message.
 * @return true If a message was retrieved.
 * @return false If no message was available.
 */
bool get_core1_msg_nowait(cmt_msg_t* msg);

/**
 * @brief Initialize the multicore environment to be ready to run the core1 functionality.
 * @ingroup mk_multicore
 *
 * This gets everything ready, but it doesn't actually start up core1. That is done by
 * `start_core1()`.
 *
 * @see start_core1()
 *
 */
void multicore_init();

/**
 * @brief Post a message to Core 0 (using the Core 0 queue). Block until it can be posted.
 * @ingroup mk_multicore
 *
 * Generally used for necessary operational information/instructions.
 *
 * @param msg The message to post.
 */
void post_to_core0_blocking(const cmt_msg_t* msg);

/**
 * @brief Post a message to Core 0 (using the Core 0 queue). Do not wait if it can't be posted.
 * @ingroup mk_multicore
 *
 * Generally used for informational status. Especially information that
 * is updated on an ongoing basis.
 *
 * @param msg The message to post.
 * @returns true if message was posted.
 */
bool post_to_core0_nowait(const cmt_msg_t* msg);

/**
 * @brief Post a message to Core 1 (using the Core 1 queue).
 * @ingroup mk_multicore
 *
 * Generally used for necessary operational information/instructions.
 *
 * @param msg The message to post.
 */
void post_to_core1_blocking(const cmt_msg_t* msg);

/**
 * @brief Post a message to Core 1 (using the Core 1 queue). Do not wait if it can't be posted.
 * @ingroup mk_multicore
 *
 * Generally used for informational status. Especially information that
 * is updated on an ongoing basis.
 *
 * @param msg The message to post.
 * @returns true if message was posted.
 */
bool post_to_core1_nowait(const cmt_msg_t* msg);

/**
 * @brief Post a message to both Core 0 and Core 1 (using the Core 0 and Core 1 queues).
 * @ingroup mk_multicore
 *
 * Generally used for necessary operational information/instructions.
 *
 * @note Since this is posting the same message to both cores, it should not be used for messages
 *       that contain allocated resources, as both core's message handlers would try to free them.
 *
 * @param msg The message to post.
 */
void post_to_cores_blocking(const cmt_msg_t* msg);

/**
 * @brief Post a message to both Core 0 and Core 1 (using the Core 0 and Core 1 queues). Do not
 *        wait if it can't be posted.
 * @ingroup mk_multicore
 *
 * Generally used for informational status. Especially information that
 * is updated on an ongoing basis.
 *
 * @note Since this is posting the same message to both cores, it should not be used for messages
 *       that contain allocated resources, as both core's message handlers would try to free them.
 *
 * @param msg The message to post.
 * @return 0 Could not post to either. 1 Posted to Core 0. 2 Posted to Core 1. 3 Posted to both cores.
 */
uint16_t post_to_cores_nowait(const cmt_msg_t* msg);

/**
 * @brief Start the Core 1 functionality.
 * @ingroup multi_core
 *
 * This starts the core1 `main` code_seq (the message dispatching loop).
 */
void start_core1();

#ifdef __cplusplus
    }
#endif
#endif // _MK_MULTICORE_H_
