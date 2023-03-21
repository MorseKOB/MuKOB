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
#include "util.h"
#include "ui_disp.h"
#include "ui_term.h"

#include "hardware/rtc.h"

#define STATUS_PULSE_PERIOD 6000

typedef struct _UI_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} ui_idle_fn_data_t;

// Message handler functions...
void _handle_send_ui_status(cmt_msg_t* msg);
void _handle_wifi_conn_status_update(cmt_msg_t* msg);
void _handle_wire_changed(cmt_msg_t* msg);
void _ui_idle_function_1(ui_idle_fn_data_t* data);

cmt_msg_t _msg_ui_send_status;
ui_idle_fn_data_t _ui_idle_function_data = { 0, 0 };

#define LEAVE_MSG_HANDLER()    {_ui_idle_function_data.idle_num = 0; _ui_idle_function_data.msg_burst++;}

msg_handler_entry_t _cmd_key_pressed_handler_entry = { MSG_CMD_KEY_PRESSED, cmd_attn_handler };
msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _ui_term_handle_init_terminal };
msg_handler_entry_t _input_char_ready_handler_entry = { MSG_INPUT_CHAR_READY, _ui_term_handle_input_char_ready };
msg_handler_entry_t _send_status_handler_entry = { MSG_SEND_UI_STATUS, _handle_send_ui_status };
msg_handler_entry_t _wifi_status_handler_entry = { MSG_WIFI_CONN_STATUS_UPDATE, _handle_wifi_conn_status_update };
msg_handler_entry_t _wire_changed_handler_entry = { MSG_WIRE_CHANGED, _handle_wire_changed };

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

void ui_init() {
    ui_disp_build();
    ui_term_build();
    cmd_init();
    // Set up to send status to BE every 800 ms
    _msg_ui_send_status.id = MSG_SEND_UI_STATUS;
    alarm_set_ms(STATUS_PULSE_PERIOD, &_msg_ui_send_status);
}

static void xyzzy() {
    // color++;
    // uint8_t fgc = fg_from_cb(color);
    // uint8_t bgc = bg_from_cb(color);
    // if (fgc == bgc) {
    //     continue;
    // }
    // screen_new();
    // disp_set_text_colors(fgc, bgc);
    // disp_clear(Paint);
    // cursor_set(19, 0);
    // test_disp_show_full_scroll_barberpoll();

    // // Test creating a new (sub) screen and writing to it
    // screen_new();
    // printf_disp(No_Paint, "This is on a new (sub)\nscreen!\n\n");
    // printf_disp(Paint, "Then it will delay. At\nthe end, pop the\nsub-screen off -\nrestoring the previous screen.");
    // sleep_ms(2000);

    // test_disp_show_mukob_head_foot();
    // disp_set_text_colors(C16_LT_GREEN, C16_BLACK);
    // disp_string(12, 0, "098765432109876543210987", false, true);
    // disp_string(13, 0, "A 1 B 2 C 3 D 4 E 5 F 6 ", false, true);
    // disp_string(14, 0, " A 1 B 2 C 3 D 4 E 5 F 6", false, true);
    // sleep_ms(1000);
    // cursor_home();
    // // void test_ili9341_show_scroll();
    // test_disp_show_half_width_scroll_barberpoll();
    // sleep_ms(1000);
    // disp_clear(Paint);
    // screen_close();
    // sleep_ms(1000);
    // disp_clear(Paint);
    // screen_close();
    // sleep_ms(1000);
}