/**
 * MuKOB Back-End - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "be.h"
#include "config.h"
#include "cmt.h"
#include "display.h"
#include "mkboard.h"
#include "mkwire.h"
#include "morse.h"
#include "net.h"
#include "util.h"
#include "hardware/rtc.h"

#define _BE_STATUS_PULSE_PERIOD 6999

typedef struct _BE_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} be_idle_fn_data_t;

// Message handler functions...
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_mks_keep_alive_send(cmt_msg_t* msg);
static void _handle_morse_decode_flush(cmt_msg_t* msg);
static void _handle_morse_to_decode(cmt_msg_t* msg);
static void _handle_send_be_status(cmt_msg_t* msg);
static void _handle_wire_connect(cmt_msg_t* msg);
static void _handle_wire_connect_toggle(cmt_msg_t* msg);
static void _handle_wire_disconnect(cmt_msg_t* msg);
static void _handle_wire_set(cmt_msg_t* msg);

// Idle functions...
static void _be_idle_function_1();
static void _be_idle_function_2();
static void _be_idle_function_3();

static cmt_msg_t _msg_be_send_status;
static be_idle_fn_data_t _msg_activity_data = { 0, 0 };

static uint32_t _last_rtc_update_ts; // ms timestamp of the last time we updated the RTC
static uint32_t _last_status_update_ts; // ms timestamp of last status update

#define LEAVE_IDLE_FUNCTION()    {_msg_activity_data.idle_num++; _msg_activity_data.msg_burst = 0;}
#define LEAVE_MSG_HANDLER()    {_msg_activity_data.idle_num = 0; _msg_activity_data.msg_burst++;}

static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _mks_keep_alive_send_handler_entry = { MSG_MKS_KEEP_ALIVE_SEND, _handle_mks_keep_alive_send };
static const msg_handler_entry_t _morse_decode_flush_handler_entry = { MSG_MORSE_DECODE_FLUSH, _handle_morse_decode_flush };
static const msg_handler_entry_t _morse_to_decode_handler_entry = { MSG_MORSE_TO_DECODE, _handle_morse_to_decode };
static const msg_handler_entry_t _send_be_status_handler_entry = { MSG_SEND_BE_STATUS, _handle_send_be_status };
static const msg_handler_entry_t _wire_connect_handler_entry = { MSG_WIRE_CONNECT, _handle_wire_connect };
static const msg_handler_entry_t _wire_connect_toggle_handler_entry = { MSG_WIRE_CONNECT_TOGGLE, _handle_wire_connect_toggle };
static const msg_handler_entry_t _wire_disconnect_handler_entry = { MSG_WIRE_DISCONNECT, _handle_wire_disconnect };
static const msg_handler_entry_t _wire_set_handler_entry = { MSG_WIRE_SET, _handle_wire_set };

// For performance - put these in order that we expect to receive more often
static const msg_handler_entry_t* _be_handler_entries[] = {
    &_morse_to_decode_handler_entry,
    &_morse_decode_flush_handler_entry,
    &_send_be_status_handler_entry,
    &_mks_keep_alive_send_handler_entry,
    &_wire_connect_handler_entry,
    &_wire_connect_toggle_handler_entry,
    &_wire_disconnect_handler_entry,
    &_wire_set_handler_entry,
    &_config_changed_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _be_idle_functions[] = {
    // Cast needed do to definition needed to avoid circular reference.
    (idle_fn)_be_idle_function_1,
    (idle_fn)_be_idle_function_2,
    (idle_fn)_be_idle_function_3,
    (idle_fn)0, // Last entry must be a NULL
};

const msg_loop_cntx_t be_msg_loop_cntx = {
    BE_CORE_NUM, // Back-end runs on Core 0
    _be_handler_entries,
    _be_idle_functions,
};

// ====================================================================
// Idle functions
// ====================================================================
static void _be_idle_function_1() {
    // Something to do when there are no messages to process.
    options_read();  // Re-read the option switches
    LEAVE_IDLE_FUNCTION();
}

static void _be_idle_function_2() {
    // Something to do when there are no messages to process.
    uint32_t now = us_to_ms(time_us_64());
    if (_last_rtc_update_ts + HOUR_IN_MS < now) {
        _last_rtc_update_ts = now;
        const config_sys_t* cfgsys = config_sys();
        network_update_rtc(cfgsys->tz_offset);
    }
    LEAVE_IDLE_FUNCTION();
}

static void _be_idle_function_3() {
    // Something to do when there are no messages to process.
    uint32_t now = us_to_ms(time_us_64());
    if (_last_status_update_ts + _BE_STATUS_PULSE_PERIOD < now) {
        // Post update status message
        _msg_be_send_status.id = MSG_SEND_BE_STATUS;
        postBEMsgNoWait(&_msg_be_send_status); // Don't wait. We will do it again in a bit.
        _last_status_update_ts = now;
    }
    LEAVE_IDLE_FUNCTION();
}


// ====================================================================
// Message handler functions
// ====================================================================

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
    morse_init(cfg->text_speed, cfg->char_speed_min, cfg->code_type, cfg->spacing);
    LEAVE_MSG_HANDLER();
}

static void _handle_mks_keep_alive_send(cmt_msg_t* msg) {
    mkwire_keep_alive_send();
    LEAVE_MSG_HANDLER();
}

static void _handle_morse_decode_flush(cmt_msg_t* msg) {
    // Call the morse decode flush function to force a decode operation to complete.
    morse_decode_flush();
    LEAVE_MSG_HANDLER();
}

/**
 * @brief Decode morse code_seq and display it in the UI.
 * @ingroup backend
 *
 * @param msg Message with `mcode_seq` to decode
 */
static void _handle_morse_to_decode(cmt_msg_t* msg) {
    mcode_seq_t* mcode_seq = msg->data.mcode_seq; // Contained code_seq and mcode_seq need to be freed when done.
    morse_decode(mcode_seq);
    mcode_seq_free(mcode_seq);
    LEAVE_MSG_HANDLER();
}

static void _handle_send_be_status(cmt_msg_t* msg) {
    // Update our status
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_disconnect(cmt_msg_t* msg) {
    mkwire_disconnect();
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_connect(cmt_msg_t* msg) {
    unsigned short wire = msg->data.wire;
    mkwire_connect(wire);
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_connect_toggle(cmt_msg_t* msg) {
    mkwire_connect_toggle();
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_set(cmt_msg_t* msg) {
    unsigned short wire = msg->data.wire;
    mkwire_wire_set(wire);
}

void be_init() {
    _last_rtc_update_ts = 0;
    char hostname[NET_URL_MAX_LEN + 1];
    const config_t* cfg = config_current();
    const char* host_and_port = cfg->host_and_port;
    uint16_t port = port_from_hostport(host_and_port, MKOBSERVER_PORT_DEFAULT);
    if (0 == host_from_hostport(hostname, NET_URL_MAX_LEN, host_and_port)) {
        mkwire_init(MKOBSERVER_DEFAULT, port, cfg->station, cfg->wire);
    }
    else {
        mkwire_init(hostname, port, cfg->station, cfg->wire);
    }
    morse_init(cfg->text_speed, cfg->char_speed_min, cfg->code_type, cfg->spacing);
}
