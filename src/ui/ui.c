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

#include <stdlib.h>
#include <string.h>

#define STATUS_PULSE_PERIOD 6000

// Internal, non message handler, function declarations
static void _ui_init_terminal_shell();

typedef struct _UI_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} ui_idle_fn_data_t;

// Message handler functions...
static void _handle_code_window_output(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_init_terminal(cmt_msg_t* msg);
static void _handle_update_ui_status(cmt_msg_t* msg);
static void _handle_wifi_conn_status_update(cmt_msg_t* msg);
static void _handle_wire_changed(cmt_msg_t* msg);
static void _handle_wire_connected_state(cmt_msg_t* msg);
static void _handle_wire_station_msgs(cmt_msg_t* msg);
static void _ui_idle_function_1(ui_idle_fn_data_t* data);

static cmt_msg_t _msg_ui_update_status;
static ui_idle_fn_data_t _ui_idle_function_data = { 0, 0 };

#define LEAVE_IDLE_FUNCTION()    {_ui_idle_function_data.idle_num++; _ui_idle_function_data.msg_burst = 0;}
#define LEAVE_MSG_HANDLER()    {_ui_idle_function_data.idle_num = 0; _ui_idle_function_data.msg_burst++;}

static const msg_handler_entry_t _cmd_key_pressed_handler_entry = { MSG_CMD_KEY_PRESSED, cmd_attn_handler };
static const msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _handle_init_terminal };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _display_in_code_window_entry = { MSG_CODE_TEXT, _handle_code_window_output };
static const msg_handler_entry_t _force_to_code_window_entry = { MSG_DISPLAY_MESSAGE, _handle_code_window_output };
static const msg_handler_entry_t _input_char_ready_handler_entry = { MSG_INPUT_CHAR_READY, _ui_term_handle_input_char_ready };
static const msg_handler_entry_t _update_status_handler_entry = { MSG_UPDATE_UI_STATUS, _handle_update_ui_status };
static const msg_handler_entry_t _wifi_status_handler_entry = { MSG_WIFI_CONN_STATUS_UPDATE, _handle_wifi_conn_status_update };
static const msg_handler_entry_t _wire_changed_handler_entry = { MSG_WIRE_CHANGED, _handle_wire_changed };
static const msg_handler_entry_t _wire_connected_state_handler_entry = { MSG_WIRE_CONNECTED_STATE, _handle_wire_connected_state };
static const msg_handler_entry_t _wire_current_sender_handler_entry = { MSG_WIRE_CURRENT_SENDER, _handle_wire_station_msgs };
static const msg_handler_entry_t _wire_station_id_handler_entry = { MSG_WIRE_STATION_ID_RCVD, _handle_wire_station_msgs };

/**
 * @brief List of handler entries.
 * @ingroup ui
 *
 * For performance, put these in the order that we expect to receive the most (most -> least).
 *
 */
static const msg_handler_entry_t* _handler_entries[] = {
    &_display_in_code_window_entry,
    &_update_status_handler_entry,
    &_input_char_ready_handler_entry,
    &_cmd_key_pressed_handler_entry,
    &_wire_current_sender_handler_entry,
    &_wire_station_id_handler_entry,
    &_wire_connected_state_handler_entry,
    &_wifi_status_handler_entry,
    &_wire_changed_handler_entry,
    &_force_to_code_window_entry,
    &_config_changed_handler_entry,
    &_cmd_init_terminal_handler_entry,
    ((msg_handler_entry_t*)0), // Last entry must be a NULL
};

static const idle_fn _ui_idle_functions[] = {
    (idle_fn)_ui_idle_function_1,
    (idle_fn)0, // Last entry must be a NULL
};

msg_loop_cntx_t ui_msg_loop_cntx = {
    UI_CORE_NUM, // UI runs on Core 1
    _handler_entries,
    _ui_idle_functions,
    &_ui_idle_function_data,
};

static char* _sender_id = NULL;

// ============================================
// Idle functions
// ============================================

static void _ui_idle_function_1(ui_idle_fn_data_t* data) {
    // Something to do when there are no messages to process.
    LEAVE_IDLE_FUNCTION();
}


// ============================================
// Message handler functions
// ============================================

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
    ui_disp_update_speed(cfg->text_speed);
    ui_term_update_speed(cfg->text_speed);
    LEAVE_MSG_HANDLER();
}

/**
 * @brief Handles MSG_CODE_TEXT and MSG_DISPLAY_MESSAGE by
 *        writing text into the code section of the display and terminal.
 * @ingroup ui
 *
 * In both cases, the data contains a string to display in the scrolling (code)
 * section of the UI. In the case of MSG_CODE_TEXT it is one or more spaces and
 * then a character. In the case of MSG_DISPLAY_MESSAGE it is a message that
 * the backend or an interrupt handler (non-UI) process wants to be displayed
 * (for example, status, warning, etc).
 *
 * @param msg The data contains a string that needs to be freed once handled.
 */
static void _handle_code_window_output(cmt_msg_t* msg) {
    char* str = msg->data.str;
    if (MSG_CODE_TEXT == msg->id) {
        // The decoder tends to put in multiple leading spaces before a character.
        // Since our screen space is limited, we reduce multiple leading spaces to one.
        // If someone actually sent multiple spaces, then they'll be lost, but most
        // of the time, it's just the decoding.
        char* s = str;
        while (strncmp(s, "  ", 2) == 0) {
            s++;
        }
        ui_disp_put_codetext(s);
        ui_term_put_codetext(s);
    }
    else {
        ui_disp_puts(str);
        ui_term_puts(str);
    }
    free(str);
    LEAVE_MSG_HANDLER();
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
static void _handle_init_terminal(cmt_msg_t* msg) {
    _ui_init_terminal_shell();
    LEAVE_MSG_HANDLER();
}

static void _handle_update_ui_status(cmt_msg_t* msg) {
    ui_disp_update_status();
    ui_term_update_status();
    // set up to do it again in a while (if not already scheduled)
    if (SCHED_MSG_ID_INVALID == scheduled_message_get(&_msg_ui_update_status)) {
        _msg_ui_update_status.id = MSG_UPDATE_UI_STATUS;
        schedule_msg_in_ms(STATUS_PULSE_PERIOD, &_msg_ui_update_status);
    }
    LEAVE_MSG_HANDLER();
}

static void _handle_wifi_conn_status_update(cmt_msg_t* msg) {
    uint32_t wifi_status = msg->data.status;
    printf_disp(Paint, "\nUI got msg: %d (%d)\n with data: %d", msg->id, _ui_idle_function_data.msg_burst + 1, wifi_status);
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_changed(cmt_msg_t* msg) {
    uint16_t wire_no = msg->data.wire;
    ui_disp_update_wire(wire_no);
    ui_term_update_wire(wire_no);
    LEAVE_MSG_HANDLER();
}

static void _handle_wire_connected_state(cmt_msg_t* msg) {
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
    LEAVE_MSG_HANDLER();
}

/**
 * Handle both messages from the wire that involve a Station ID.
 */
static void _handle_wire_station_msgs(cmt_msg_t* msg) {
    char* msg_station_id = msg->data.station_id;

    if (MSG_WIRE_CURRENT_SENDER == msg->id) {
        if (_sender_id) {
            if (strcmp(_sender_id, msg_station_id) == 0) {
                // Same station, just free the message station id
                free(msg_station_id);
                return;
            }
            free(_sender_id);
        }
        // Different station, store it and update the UI.
        _sender_id = msg->data.station_id;
        ui_disp_update_sender(_sender_id);
        ui_term_update_sender(_sender_id);
    }
    else if (MSG_WIRE_STATION_ID_RCVD == msg->id) {
        // Update the station list
        // ZZZ for now, just free up the string.
        free(msg->data.station_id);
    }
    LEAVE_MSG_HANDLER();
}


// ============================================
// Internal functions
// ============================================

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

// ============================================
// Public functions
// ============================================

void ui_init() {
    ui_disp_build();
    _ui_init_terminal_shell();
    wire_connected_state_t connected_state = mkwire_connected_state();
    cmt_msg_t msg;
    msg.id = MSG_WIRE_CONNECTED_STATE;
    msg.data.status = connected_state;
    postUIMsgBlocking(&msg);

    // Set up to update status every 6 seconds
    _msg_ui_update_status.id = MSG_UPDATE_UI_STATUS;
    schedule_msg_in_ms(STATUS_PULSE_PERIOD, &_msg_ui_update_status);
}
