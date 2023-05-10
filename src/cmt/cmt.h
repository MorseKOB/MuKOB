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
    MSG_DEBUG_CHANGED,
    //
    // Back-End messages
    MSG_BACKEND_NOOP = 0x0100,
    MSG_BE_TEST,
    MSG_CMT_SLEEP,
    MSG_KEY_READ,
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
    MSG_UI_NOOP = 0x0200,
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
typedef void (*cmt_sleep_fn)(void* user_data);

typedef struct _cmt_sleep_data_ {
    cmt_sleep_fn sleep_fn;
    void* user_data;
} _cmt_sleep_data_t;

/**
 * @brief Message data.
 *
 * Union that can hold the data needed by the messages.
 */
typedef union _MSG_DATA_VALUE {
    char c;
    bool debug;
    uint32_t ts_ms;
    uint64_t ts_us;
    key_read_state_t key_read_state;
    kob_status_t kob_status;
    mcode_seq_t* mcode_seq;
    _cmt_sleep_data_t* cmt_sleep;
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
 * @param t The millisecond time msg was posted (set by the posting system)
 */
typedef struct _CMT_MSG {
    msg_id_t id;
    msg_data_value_t data;
    uint32_t t;
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

typedef struct _PROC_STATUS_ACCUM_ {
    volatile int64_t cs;
    volatile uint32_t ts_psa;                               // Timestamp of last PS Accumulator/sec update
    volatile uint32_t t_active;
    volatile uint32_t t_idle;
    volatile uint32_t t_msgr;
    volatile uint16_t retrived;
    volatile uint16_t idle;
    volatile uint32_t int_status;
    volatile float core_temp;
} proc_status_accum_t;

typedef struct _MSG_LOOP_CNTX {
    uint8_t corenum;                                // The core number the loop is running on
    const msg_handler_entry_t** handler_entries;    // NULL terminated list of message handler entries
    const idle_fn* idle_functions;                  // Null terminated list of idle functions
} msg_loop_cntx_t;

/**
 * @brief Indicates if the Core-0 message loop has been started.
 * @ingroup cmt
 *
 * @return true The Core-0 message loop has been started.
 * @return false The Core-0 message loop has not been started yet.
 */
extern bool cmt_message_loop_0_running();

/**
 * @brief Indicates if the Core-1 message loop has been started.
 * @ingroup cmt
 *
 * @return true The Core-1 message loop has been started.
 * @return false The Core-1 message loop has not been started yet.
 */
extern bool cmt_message_loop_1_running();

/**
 * @brief Indicates if both the Core-0 and Core-1 message loops have been started.
 * @ingroup cmt
 *
 * @return true The message loops have been started.
 * @return false The message loops have not been started yet.
 */
extern bool cmt_message_loops_running();

/**
 * @brief Handle a Scheduled Message timer Tick.
 *
 * @param msg (not used)
 */
extern void cmt_handle_sleep(cmt_msg_t* msg);

/**
 * @brief Get the last Process Status Accumulator per second values.
 *
 * @param psas Pointer to Process Status Accumulator structure to fill with values.
 * @param corenum The core number (0|1) to get the process status values for.
 */
extern void cmt_proc_status_sec(proc_status_accum_t* psas, uint8_t corenum);

/**
 * @brief The number of scheduled messages waiting.
 *
 * @return int Number of scheduled messages.
 */
extern int cmt_sched_msg_waiting();

/**
 * @brief Sleep for milliseconds and call a function.
 * @ingroup cmt
 *
 * @param ms The time in milliseconds from now.
 * @param sleep_fn The function to call when the time expires.
 * @param user_data A pointer to user data that the 'sleep_fn' will be called with.
 */
extern void cmt_sleep_ms(int32_t ms, cmt_sleep_fn sleep_fn, void* user_data);

/**
 * @brief Schedule a message to post in the future.
 *
 * @param ms The time in milliseconds from now.
 * @param msg The cmt_msg_t message to post when the time period elapses.
 */
extern void schedule_msg_in_ms(int32_t ms, cmt_msg_t* msg);

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
extern void cmt_module_init();

#ifdef __cplusplus
    }
#endif
#endif // _MK_CMT_H_
