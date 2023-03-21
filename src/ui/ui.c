/**
 * MuKOB User Interface - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/

#include "ui.h"
#include "cmd.h"
#include "cmt.h"
#include "display.h"
#include "mkwire.h"
#include "util.h"
#include "ui_disp.h"
#include "ui_term.h"

#include "hardware/rtc.h"

#define STATUS_PULSE_PERIOD 6000

// Internal, non message handler, function declarations
static void _ui_init_terminal_shell();

typedef struct _UI_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} ui_idle_fn_data_t;

// Message handler functions...
void _handle_init_terminal(cmt_msg_t* msg);
void _handle_send_ui_status(cmt_msg_t* msg);
void _handle_wifi_conn_status_update(cmt_msg_t* msg);
void _handle_wire_changed(cmt_msg_t* msg);
void _handle_wire_connected_state(cmt_msg_t* msg);
void _ui_idle_function_1(ui_idle_fn_data_t* data);

cmt_msg_t _msg_ui_send_status;
ui_idle_fn_data_t _ui_idle_function_data = { 0, 0 };

#define LEAVE_MSG_HANDLER()    {_ui_idle_function_data.idle_num = 0; _ui_idle_function_data.msg_burst++;}

msg_handler_entry_t _cmd_key_pressed_handler_entry = { MSG_CMD_KEY_PRESSED, cmd_attn_handler };
msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _handle_init_terminal };
msg_handler_entry_t _input_char_ready_handler_entry = { MSG_INPUT_CHAR_READY, _ui_term_handle_input_char_ready };
msg_handler_entry_t _send_status_handler_entry = { MSG_SEND_UI_STATUS, _handle_send_ui_status };
msg_handler_entry_t _wifi_status_handler_entry = { MSG_WIFI_CONN_STATUS_UPDATE, _handle_wifi_conn_status_update };
msg_handler_entry_t _wire_changed_handler_entry = { MSG_WIRE_CHANGED, _handle_wire_changed };
msg_handler_entry_t _wire_connected_state_handler_entry = { MSG_WIRE_CONNECTED_STATE, _handle_wire_connected_state };

/**
 * @brief List of handler entries.
 * @ingroup ui
 *
 * For performance, put these in the order that we expect to receive the most (most -> least).
 *
 */
msg_handler_entry_t* _handler_entries[] = {
    &_send_status_handler_entry,
    &_input_char_ready_handler_entry,
    &_cmd_key_pressed_handler_entry,
    &_wire_connected_state_handler_entry,
    &_wifi_status_handler_entry,
    &_wire_changed_handler_entry,
    &_cmd_init_terminal_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

idle_fn _ui_idle_functions[] = {
    (void (*)(void*))_ui_idle_function_1,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t ui_msg_loop_cntx = {
    UI_CORE_NUM, // UI runs on Core 1
    _handler_entries,
    _ui_idle_functions,
    &_ui_idle_function_data,
};


void _ui_idle_function_1(ui_idle_fn_data_t* data) {
    // Something to do when there are no messages to process.
    data->msg_burst = 0; // Reset our message burst count
    data->idle_num++;
}

/**
 * @brief Message handler for MSG_INIT_TERMINAL
 * @ingroup ui
 *
 * Init/re-init the terminal. This is typically received by a user requesting
 * that the terminal be re-initialized/refreshed. For example if they connect
 * a terminal after MuKOB is already up and running.
 *
 * @param msg Nothing important in the message.
 */
void _handle_init_terminal(cmt_msg_t* msg) {
    _ui_init_terminal_shell();
}

cmt_msg_t _send_ui_status_msg;
void _handle_send_ui_status(cmt_msg_t* msg) {
    // Post a message
    _send_ui_status_msg.id = MSG_BACKEND_NOOP;
    postBEMsgNoWait(&_send_ui_status_msg);
    // Send status to BE in a while
    _msg_ui_send_status.id = MSG_SEND_UI_STATUS;
    alarm_set_ms(STATUS_PULSE_PERIOD, &_msg_ui_send_status);
    ui_disp_update_status();
    ui_term_update_status();
}

void _handle_wifi_conn_status_update(cmt_msg_t* msg) {
    uint32_t wifi_status = msg->data.status;
    printf_disp(Paint, "\nUI got msg: %d (%d)\n with data: %d", msg->id, _ui_idle_function_data.msg_burst + 1, wifi_status);
    LEAVE_MSG_HANDLER();
}

void _handle_wire_changed(cmt_msg_t* msg) {
    uint16_t wire_no = msg->data.wire;
    ui_disp_update_wire(wire_no);
    ui_term_update_wire(wire_no);
}

void _handle_wire_connected_state(cmt_msg_t* msg) {
    wire_connected_state_t state = (wire_connected_state_t)msg->data.status;
    ui_disp_update_connected_state(state);
    ui_term_update_connected_state(state);

    // If connected, cancel out of an active command shell.
    if (WIRE_CONNECTED == state) {
        cmd_enter_idle_state();
    }
    else {
        // Make sure a command shell is available.
        if (CMD_SNOOZING == cmd_get_state()) {
            // Do this by posting a message.
            cmt_msg_t msg;
            msg.id = MSG_CMD_KEY_PRESSED;
            msg.data.c = CMD_WAKEUP_CHAR;
            postUIMsgBlocking(&msg);
        }
    }
}

static void _ui_init_terminal_shell() {
    ui_term_build();
    cmd_init();
    // If we aren't connected to a wire, enter into the command shell by posting a message.
    if (!mkwire_is_connected()) {
        // Do this by posting a message.
        cmt_msg_t msg;
        msg.id = MSG_CMD_KEY_PRESSED;
        msg.data.c = CMD_WAKEUP_CHAR;
        postUIMsgBlocking(&msg);
    }
}

void ui_init() {
    ui_disp_build();
    _ui_init_terminal_shell();
    wire_connected_state_t connected_state = mkwire_connected_state();
    cmt_msg_t msg;
    msg.id = MSG_WIRE_CONNECTED_STATE;
    msg.data.status = connected_state;
    postUIMsgBlocking(&msg);

    // Set up to send status to BE every 800 ms
    _msg_ui_send_status.id = MSG_SEND_UI_STATUS;
    alarm_set_ms(STATUS_PULSE_PERIOD, &_msg_ui_send_status);
}
