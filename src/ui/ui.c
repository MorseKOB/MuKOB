/**
 * MuKOB User Interface - Base.
 *
 * Setup for the message loop and idle processing.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "system_defs.h"
#include "ui.h"

#include "cmd.h"
#include "cmt.h"
#include "core1_main.h"
#include "display.h"
#include "mkboard.h"
#include "mkwire.h"
#include "multicore.h"
#include "re_pbsw.h"
#include "rotary_encoder.h"
#include "touch.h"
#include "util.h"
#include "ui_disp.h"
#include "ui_term.h"

#include "hardware/rtc.h"

#include <stdlib.h>
#include <string.h>

#define _UI_STATUS_PULSE_PERIOD 7001

// Internal, non message handler, function declarations
static void _sort_station_list(const mk_station_id_t *stations[], int len);
static void _ui_init_terminal_shell();

// Message handler functions...
static void _handle_be_initialized(cmt_msg_t* msg);
static void _handle_code_window_output(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_init_terminal(cmt_msg_t* msg);
static void _handle_kob_status(cmt_msg_t* msg);
static void _handle_touch_panel(cmt_msg_t* msg);
static void _handle_update_ui_status(cmt_msg_t* msg);
static void _handle_wifi_conn_status_update(cmt_msg_t* msg);
static void _handle_wire_changed(cmt_msg_t* msg);
static void _handle_wire_connected_state(cmt_msg_t* msg);
static void _handle_wire_station_msgs(cmt_msg_t* msg);

static void _ui_idle_function_1();

static cmt_msg_t _msg_ui_cmd_start;
static cmt_msg_t _msg_ui_initialized;
static cmt_msg_t _msg_ui_update_status;
static cmt_msg_t _msg_ui_update_wire;

static uint32_t _last_status_update_ts; // ms timestamp of last status update

static const msg_handler_entry_t _be_initialized_handler_entry = { MSG_BE_INITIALIZED, _handle_be_initialized };
static const msg_handler_entry_t _cmd_key_pressed_handler_entry = { MSG_CMD_KEY_PRESSED, cmd_attn_handler };
static const msg_handler_entry_t _cmd_init_terminal_handler_entry = { MSG_CMD_INIT_TERMINAL, _handle_init_terminal };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _display_in_code_window_entry = { MSG_CODE_TEXT, _handle_code_window_output };
static const msg_handler_entry_t _force_to_code_window_entry = { MSG_DISPLAY_MESSAGE, _handle_code_window_output };
static const msg_handler_entry_t _input_char_ready_handler_entry = { MSG_INPUT_CHAR_READY, _ui_term_handle_input_char_ready };
static const msg_handler_entry_t _kob_status_handler_entry = { MSG_KOB_STATUS, _handle_kob_status };
static const msg_handler_entry_t _touch_panel_handler_entry = { MSG_TOUCH_PANEL, _handle_touch_panel };
static const msg_handler_entry_t _update_status_handler_entry = { MSG_UPDATE_UI_STATUS, _handle_update_ui_status };
static const msg_handler_entry_t _wifi_status_handler_entry = { MSG_WIFI_CONN_STATUS_UPDATE, _handle_wifi_conn_status_update };
static const msg_handler_entry_t _wire_changed_handler_entry = { MSG_WIRE_CHANGED, _handle_wire_changed };
static const msg_handler_entry_t _wire_connected_state_handler_entry = { MSG_WIRE_CONNECTED_STATE, _handle_wire_connected_state };
static const msg_handler_entry_t _wire_current_sender_handler_entry = { MSG_WIRE_CURRENT_SENDER, _handle_wire_station_msgs };
static const msg_handler_entry_t _wire_station_id_handler_entry = { MSG_WIRE_STATION_ID_RCVD, _handle_wire_station_msgs };
static const msg_handler_entry_t _wire_stations_cleared_handler_entry = { MSG_WIRE_STATIONS_CLEARED, _handle_wire_station_msgs };

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
    &_kob_status_handler_entry,
    &_cmd_key_pressed_handler_entry,
    &_touch_panel_handler_entry,
    &_wire_current_sender_handler_entry,
    &_wire_station_id_handler_entry,
    &_wire_stations_cleared_handler_entry,
    &_wire_connected_state_handler_entry,
    &_wifi_status_handler_entry,
    &_wire_changed_handler_entry,
    &_force_to_code_window_entry,
    &_config_changed_handler_entry,
    &_cmd_init_terminal_handler_entry,
    &_be_initialized_handler_entry,
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
};

static const char* _sender_id = NULL;

// ============================================
// Idle functions
// ============================================

static void _ui_idle_function_1() {
    // Something to do when there are no messages to process.
    uint32_t now = now_ms();

    if (_last_status_update_ts + _UI_STATUS_PULSE_PERIOD < now) {
        // Post update status message
        _msg_ui_update_status.id = MSG_UPDATE_UI_STATUS;
        postUIMsgNoWait(&_msg_ui_update_status); // Don't wait. We will do it again in a bit.
        _last_status_update_ts = now;
    }
}


// ============================================
// Message handler functions
// ============================================

static void _handle_be_initialized(cmt_msg_t* msg) {
    // The Backend has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
    //
    // Update the wire-connected indicator.
    wire_connected_state_t connected_state = mkwire_connected_state();
    _msg_ui_update_wire.id = MSG_WIRE_CONNECTED_STATE;
    _msg_ui_update_wire.data.status = connected_state;
    postUIMsgBlocking(&_msg_ui_update_wire);
    // If we aren't connected to a wire, enter into the command shell.
    if (!mkwire_is_connected()) {
        // Do this by posting a message.
        _msg_ui_cmd_start.id = MSG_CMD_KEY_PRESSED;
        _msg_ui_cmd_start.data.c = CMD_WAKEUP_CHAR;
        postUIMsgBlocking(&_msg_ui_cmd_start);
    }
}

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
    ui_disp_update_speed(cfg->text_speed);
    ui_term_update_speed(cfg->text_speed);
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
}

static void _handle_kob_status(cmt_msg_t* msg) {
    ui_disp_update_kob_status(&(msg->data.kob_status));
    ui_term_update_kob_status(&(msg->data.kob_status));
}

static void _handle_touch_panel(cmt_msg_t* msg) {
    gfx_point *sp = msg->data.touch_point;
    gfx_point *pp = tp_last_panel_point();
    uint16_t f = tp_last_touch_force();
    debug_printf(false, "Touch - Screen:%3.3d,%3.3d  Panel:%5.5d,%5.5d  Force:%5.5u\n", sp->x, sp->y, pp->x, pp->y, f);
}

static void _handle_update_ui_status(cmt_msg_t* msg) {
    ui_disp_update_status();
    ui_term_update_status();
}

static void _handle_wifi_conn_status_update(cmt_msg_t* msg) {
    uint32_t wifi_status = msg->data.status;
    debug_printf(true, "UI - Update wifi status: %u\n", wifi_status);
}

static void _handle_wire_changed(cmt_msg_t* msg) {
    uint16_t wire_no = msg->data.wire;
    ui_disp_update_wire(wire_no);
    ui_term_update_wire(wire_no);
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
            // Do this by posting a message as though the wakeup key was pressed.
            cmt_msg_t msg;
            msg.id = MSG_CMD_KEY_PRESSED;
            msg.data.c = CMD_WAKEUP_CHAR;
            postUIMsgBlocking(&msg);
        }
    }
}

/**
 * Handle all messages from the wire that involve a Station ID.
 */
static void _handle_wire_station_msgs(cmt_msg_t *msg) {
    const char* msg_station_id = msg->data.station_id;

    if (MSG_WIRE_CURRENT_SENDER == msg->id) {
        if (_sender_id) {
            if (strcmp(_sender_id, msg_station_id) == 0) {
                return;
            }
        }
        // Different station, store it and update the UI.
        _sender_id = msg->data.station_id;
        ui_disp_update_sender(_sender_id);
        ui_term_update_sender(_sender_id);
    }
    else if (MSG_WIRE_STATION_ID_RCVD == msg->id) {
        const mk_station_id_t **stations = mkwire_active_stations();
        // Remove current sender and sort.
        int count = 0;
        while (*(stations + count)) {
            count++;
        }
        const mk_station_id_t *ss[count];
        const mk_station_id_t **ssp = ss;
        int sc = 0;
        // Don't include the current sender.
        for (int i = 0; i < count; i++) {
            const mk_station_id_t *station = *stations++;
            if (strcmp(_sender_id, station->id) != 0) {
                *ssp++ = station;
                sc++;
            }
        }
        ss[sc] = (mk_station_id_t*)0; // Mark end with NULL
        _sort_station_list(ss, sc);
        ui_disp_update_stations(ss, sc);
        ui_term_update_stations(ss, sc);
    }
    else if (MSG_WIRE_STATIONS_CLEARED == msg->id) {
        // Remove all of the stations and the current sender.
        _sender_id = NULL;
        ui_disp_update_sender(_sender_id);
        ui_term_update_sender(_sender_id);
        const mk_station_id_t** stations = mkwire_active_stations();
        ui_disp_update_stations(stations, 0);
        ui_term_update_stations(stations, 0);
    }
}


// ============================================
// Internal functions
// ============================================

static void _qsort_swap(void *v[], int i, int j) {
    void *temp;

    temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

static int _qsort_sl_comp(const mk_station_id_t *s1, const mk_station_id_t *s2) {
    // Sort by last received from (longest first).
    // If we haven't received from the station, sort by time first seen.
    if (s1->ts_recv != 0 || s2->ts_recv != 0) {
        // We have received from one or the other or both of them.
        if (s1->ts_recv != 0 && s2->ts_recv == 0) {
            return -1;
        }
        else if (s1->ts_recv == 0 && s2->ts_recv != 0) {
            return 1;
        }
        else if (s1->ts_recv < s2->ts_recv) {
            return -1;
        }
        else if (s1->ts_recv > s2->ts_recv) {
            return 1;
        }
    }
    // We haven't received from either of them (or received from both at the same time?).
    if (s1->ts_init < s2->ts_init) {
        return -1;
    }
    else if (s1->ts_init > s2->ts_init) {
        return 1;
    }
    return 0;
}

static void _qsort_sl(const mk_station_id_t *s[], int left, int right) {
    int last;

    // Do nothing if array contains fewer than two elements.
    if (left >= right) {
        return;
    }
    _qsort_swap((void **)s, left, (left + right) / 2);
    last = left;
    for (int i = left+1; i <= right; i++) {
        if (_qsort_sl_comp(s[i], s[left]) < 0) {
            _qsort_swap((void **)s, ++last, i);
        }
    }
    _qsort_swap((void**)s, left, last);
    _qsort_sl(s, left, last-1);
    _qsort_sl(s, last+1, right);
}

static void _sort_station_list(const mk_station_id_t *stations[], int len) {
    _qsort_sl(stations, 0, len - 1);
}

static void _ui_init_terminal_shell() {
    ui_term_build();
    cmd_module_init();
}

void _ui_gpio_irq_handler(uint gpio, uint32_t events) {
    switch (gpio) {
        case IRQ_rotary_SW:
            re_pbsw_irq_handler(gpio, events);
            break;
        case IRQ_rotary_TURN:
            re_turn_irq_handler(gpio, events);
            break;
        case IRQ_TOUCH:
            gpio_set_irq_enabled(IRQ_TOUCH, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
            tp_irq_handler(gpio, events);
            gpio_set_irq_enabled(IRQ_TOUCH, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
            break;
    }
}


// ============================================
// Public functions
// ============================================

void start_ui(void) {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;
    start_core1(); // The Core-1 main starts the UI
}

void ui_module_init() {
    re_pbsw_module_init();
    rotary_encoder_module_init();
    gpio_set_irq_enabled_with_callback(IRQ_rotary_TURN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &_ui_gpio_irq_handler);
    gpio_set_irq_enabled(IRQ_rotary_SW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(IRQ_TOUCH, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    ui_disp_build();
    _ui_init_terminal_shell();
    // Let the Backend know that we are initialized
    _msg_ui_initialized.id = MSG_UI_INITIALIZED;
    postBEMsgBlocking(&_msg_ui_initialized);
}
