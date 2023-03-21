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
#include "mkboard.h"
#include "mkwire.h"
#include "net.h"
#include "util.h"
#include "hardware/rtc.h"

#define STATUS_PULSE_PERIOD 8000

typedef struct _BE_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} be_idle_fn_data_t;

// Internal functions...
void _handle_send_be_status(cmt_msg_t* msg);
void _handle_wire_connect(cmt_msg_t* msg);
void _handle_wire_connect_toggle(cmt_msg_t* msg);
void _handle_wire_disconnect(cmt_msg_t* msg);
void _handle_wire_set(cmt_msg_t* msg);
void _be_idle_function_1(be_idle_fn_data_t* data);

cmt_msg_t _msg_be_send_status;
be_idle_fn_data_t _be_idle_function_data = { 0, 0 };

#define LEAVE_MSG_HANDLER()    {_be_idle_function_data.idle_num = 0; _be_idle_function_data.msg_burst++;}

msg_handler_entry_t _send_be_status_handler_entry = { MSG_SEND_BE_STATUS, _handle_send_be_status };
msg_handler_entry_t _wire_connect_handler_entry = { MSG_WIRE_CONNECT, _handle_wire_connect };
msg_handler_entry_t _wire_connect_toggle_handler_entry = { MSG_WIRE_CONNECT_TOGGLE, _handle_wire_connect_toggle };
msg_handler_entry_t _wire_disconnect_handler_entry = { MSG_WIRE_DISCONNECT, _handle_wire_disconnect };
msg_handler_entry_t _wire_set_handler_entry = { MSG_WIRE_SET, _handle_wire_set };

msg_handler_entry_t* _be_handler_entries[] = {
    &_send_be_status_handler_entry,
    &_wire_connect_handler_entry,
    &_wire_connect_toggle_handler_entry,
    &_wire_disconnect_handler_entry,
    &_wire_set_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

idle_fn _be_idle_functions[] = {
    (void (*)(void*))_be_idle_function_1,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t be_msg_loop_cntx = {
    BE_CORE_NUM, // Back-end runs on Core 0
    _be_handler_entries,
    _be_idle_functions,
    &_be_idle_function_data,
};


void _be_idle_function_1(be_idle_fn_data_t* data) {
    // Something to do when there are no messages to process.
    options_read();  // Re-read the option switches
    data->msg_burst = 0; // Reset our message burst count
    data->idle_num++;
}

cmt_msg_t _send_be_status_msg;
void _handle_send_be_status(cmt_msg_t* msg) {
    // Post a message
    _send_be_status_msg.id = MSG_UI_NOOP;
    postUIMsgNoWait(&_send_be_status_msg);
    // Set up to send status in a while
    _msg_be_send_status.id = MSG_SEND_BE_STATUS;
    alarm_set_ms(500, &_msg_be_send_status);
    LEAVE_MSG_HANDLER();
}

void _handle_wire_disconnect(cmt_msg_t* msg) {
    mkwire_disconnect();
    LEAVE_MSG_HANDLER();
}

void _handle_wire_connect(cmt_msg_t* msg) {
    unsigned short wire = msg->data.wire;
    mkwire_connect(wire);
    LEAVE_MSG_HANDLER();
}

void _handle_wire_connect_toggle(cmt_msg_t* msg) {
    mkwire_connect_toggle();
    LEAVE_MSG_HANDLER();
}

void _handle_wire_set(cmt_msg_t* msg) {
    unsigned short wire = msg->data.wire;
    mkwire_wire_set(wire);
}

void be_init() {
    char host[NET_URL_MAX_LEN + 1];
    const config_t* cfg = config_current();
    const char* host_port = cfg->host_port;
    uint16_t port = mkwire_port_from_hostport(host_port);
    mkwire_host_from_hostport(host, NET_URL_MAX_LEN, host_port);
    mkwire_init(host, port, cfg->station, cfg->wire);

    // Set up to send status to UI regularly
    _msg_be_send_status.id = MSG_SEND_BE_STATUS;
    alarm_set_ms(STATUS_PULSE_PERIOD, &_msg_be_send_status);
}
