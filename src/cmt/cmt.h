/**
 * MuKOB Cooperative Multi-Tasking.
 *
 * Containes message loop, timer, and other CMT enablement functions.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _MK_CMT_H_
#define _MK_CMT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "pico/types.h"

typedef enum _MSG_ID_ {
    MSG_COMMON_NOOP = 0x0000,   // Common messages
    MSG_DELAY_COMPLETE,
    //
    MSG_BACKEND_NOOP = 0x4000,  // Back-end messages
    MSG_WIRE_CONNECT,
    MSG_WIRE_DISCONNECT,
    MSG_SEND_BE_STATUS,
    //
    MSG_UI_NOOP = 0x8000,       // UI messages
    MSG_CMD_KEY_PRESSED,
    MSG_INPUT_CHAR_READY,
    MSG_SEND_UI_STATUS,
    MSG_WIFI_CONN_STATUS_UPDATE,
} msg_id_t;

/**
 * @brief Message data.
 *
 * Union that can hold the data needed by the messages.
 */
typedef union _MSG_DATA_VALUE {
    // General purpose (non-specifically named) values
    char c;
    unsigned char uc;
    int16_t n;
    uint16_t un;
    int32_t d;
    uint32_t ud;
    int64_t l;
    uint64_t ll;
    char* str;
    void* ex_data;
    // Specific values (more readable, for use by specific messages)
    int32_t status;
    unsigned short wire;
} msg_data_value_t;

/**
 * @brief Structure containing a message ID and message data.
 *
 * @param id The ID (number) of the message.
 * @param data The data for the message.
 */
typedef struct _CMT_MSG {
    msg_id_t id;
    msg_data_value_t data;
} cmt_msg_t;

#include "multicore.h"

// Define functional names for the 'Core' message queue functions (Camel-case to help flag as aliases).
#define getBEMsgBlocking( pmsg )        get_core1_msg_blocking( pmsg )
#define getBEMsgNoWait( pmsg )          get_core1_msg_nowait( pmsg )
#define getUIMsgBlocking( pmsg )        get_core0_msg_blocking( pmsg )
#define getUIMsgNoWait( pmsg )          get_core0_msg_nowait( pmsg )
#define postBEMsgBlocking( pmsg )       post_to_core0_blocking( pmsg )
#define postBEMsgNoWait( pmsg )         post_to_core0_nowait( pmsg )
#define postUIMsgBlocking( pmsg )       post_to_core1_blocking( pmsg )
#define postUIMsgNoWait( pmsg )         post_to_core1_nowait( pmsg )

/**
 * @brief Function prototype for a message handler.
 * @ingroup cmt
 *
 * @param cntx The message loop context.
 */
typedef void (*idle_fn)(void* data);

/**
 * @brief Function prototype for a message handler.
 * @ingroup cmt
 *
 * @param msg The message to handle.
 */
typedef void (*msg_handler_fn)(cmt_msg_t* msg);


typedef struct _MSG_HANDLER_ENTRY {
    int msg_id;
    msg_handler_fn msg_handler;
} msg_handler_entry_t;

typedef struct _MSG_DISPATCH_CNTX {
} msg_dispatch_cntx_t;

typedef struct _MSG_LOOP_CNTX {
    uint8_t corenum;                        // The core number the loop is running on
    msg_handler_entry_t** handler_entries;  // NULL terminated list of message handler entries
    idle_fn* idle_functions;                // Null terminated list of idle functions
    void* idle_data;                        // Data to pass to the idle functions
} msg_loop_cntx_t;

/**
 * @brief Set an alarm to post a message when a millisecond period elapses.
 *
 * @param ms The time in milliseconds.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 */
extern void alarm_set_ms(uint32_t ms, cmt_msg_t* msg);

/**
 * @brief Enter into a message processing loop.
 * @ingroup cmt
 *
 * Enter into a message processing loop using a loop context.
 * This function will not return.
 *
 * @param loop_context Loop context for processing.
 */
extern void enter_message_loop(msg_loop_cntx_t* loop_context);

#ifdef __cplusplus
    }
#endif
#endif // _MK_CMT_H_
