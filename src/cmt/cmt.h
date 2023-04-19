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
#include "kob_t.h"
#include "morse.h"
#include "pico/types.h"

typedef enum _MSG_ID_ {
    // Common messages (used by both BE and UI)
    MSG_COMMON_NOOP = 0x0000,
    MSG_CONFIG_CHANGED,
    //
    // Back-End messages
    MSG_BACKEND_NOOP = 0x4000,
    MSG_CMT_SLEEP,
    MSG_KOB_KEY_READ,
    MSG_KOB_SOUND_CODE_CONT,
    MSG_MKS_KEEP_ALIVE_SEND,
    MSG_MKS_PACKET_RECEIVED,
    MSG_MORSE_DECODE_FLUSH,
    MSG_MORSE_CODE_SEQUENCE,
    MSG_SEND_BE_STATUS,
    MSG_UI_INITIALIZED,
    MSG_WIRE_CONNECT,
    MSG_WIRE_CONNECT_TOGGLE,
    MSG_WIRE_DISCONNECT,
    MSG_WIRE_SET,
    //
    // Front-End/UI messages
    MSG_UI_NOOP = 0x8000,
    MSG_BE_INITIALIZED,
    MSG_CMD_KEY_PRESSED,
    MSG_CMD_INIT_TERMINAL,
    MSG_INPUT_CHAR_READY,
    MSG_CODE_TEXT,
    MSG_DISPLAY_MESSAGE,
    MSG_KOB_STATUS,
    MSG_UPDATE_UI_STATUS,
    MSG_WIFI_CONN_STATUS_UPDATE,
    MSG_WIRE_CHANGED,
    MSG_WIRE_CONNECTED_STATE,
    MSG_WIRE_CURRENT_SENDER,
    MSG_WIRE_STATION_ID_RCVD,
} msg_id_t;

/**
 * @brief Function prototype for a sleep function.
 * @ingroup cmt
 */
typedef void (*cmt_sleep_fn)(void);

/**
 * @brief Message data.
 *
 * Union that can hold the data needed by the messages.
 */
typedef union _MSG_DATA_VALUE {
    char c;
    uint32_t ts_ms;
    uint64_t ts_us;
    key_read_state_t key_read_state;
    kob_status_t kob_status;
    mcode_seq_t* mcode_seq;
    cmt_sleep_fn* sleep_fn;
    const char* station_id;
    char* str;
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
#define postBothMsgBlocking( pmsg )     post_to_cores_blocking( pmsg )
#define postBothMsgNoWait( pmsg )       post_to_cores_nowait( pmsg )

/**
 * @brief Function prototype for an idle function.
 * @ingroup cmt
 */
typedef void (*idle_fn)(void);

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
    uint8_t corenum;                                // The core number the loop is running on
    const msg_handler_entry_t** handler_entries;    // NULL terminated list of message handler entries
    const idle_fn* idle_functions;                  // Null terminated list of idle functions
} msg_loop_cntx_t;

/**
 * @brief Handle a Scheduled Message timer Tick.
 *
 * @param msg (not used)
 */
void cmt_handle_sleep(cmt_msg_t* msg);

/**
 * @brief Sleep for milliseconds and call a function.
 * @ingroup cmt
 *
 * @param ms The time in milliseconds from now.
 * @param sleep_fn The function to call when the time expires.
 */
void cmt_sleep_ms(int32_t ms, cmt_sleep_fn* sleep_fn);

/**
 * @brief Schedule a message to post in the future.
 *
 * @param ms The time in milliseconds from now.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 *
 * @return True if it could be scheduled.
 */
extern bool schedule_msg_in_ms(int32_t ms, const cmt_msg_t* msg);

/**
 * @brief Cancel scheduled message(s) for a message ID.
 * @ingroup cmt
 *
 * This will attempt to cancel the scheduled message. It is possible that the time might have already
 * past and the message was posted.
 *
 * @param sched_msg_id The ID of the message that was scheduled.
 */
extern void scheduled_msg_cancel(msg_id_t sched_msg_id);

/**
 * @brief Get the ID of a scheduled message if one exists.
 * @ingroup cmt
 *
 * Typically, this is used to keep from adding a scheduled message if one already exists.
 *
 * @param sched_msg_id The ID of the message to check for.
 * @return True if there is a scheduled message for the ID.
 */
extern bool scheduled_message_exists(msg_id_t sched_msg_id);

/**
 * @brief Enter into a message processing loop.
 * @ingroup cmt
 *
 * Enter into a message processing loop using a loop context.
 * This function will not return.
 *
 * @param loop_context Loop context for processing.
 */
extern void message_loop(const msg_loop_cntx_t* loop_context);

/**
 * @brief Initialize the Cooperative Multi-Tasking system.
 * @ingroup cmt
 */
void cmt_module_init();

#ifdef __cplusplus
    }
#endif
#endif // _MK_CMT_H_
