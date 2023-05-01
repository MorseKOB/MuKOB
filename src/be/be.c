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
#include "kob.h"
#include "mkboard.h"
#include "mkdebug.h"
#include "mkwire.h"
#include "morse.h"
#include "net.h"
#include "util.h"
#include "hardware/rtc.h"

#define _BE_STATUS_PULSE_PERIOD 6999

static config_t* _last_cfg;
static cmt_msg_t _msg_wire_set = { MSG_WIRE_SET, {0} };

typedef struct _BE_IDLE_FN_DATA_ {
    unsigned long int idle_num;
    unsigned long int msg_burst;
} be_idle_fn_data_t;

// Message handler functions...
static void _handle_be_test(cmt_msg_t* msg);
static void _handle_config_changed(cmt_msg_t* msg);
static void _handle_cmt_sleep(cmt_msg_t* msg);
static void _handle_kob_key_read(cmt_msg_t* msg);
static void _handle_kob_sound_code_cont(cmt_msg_t* msg);
static void _handle_mks_keep_alive_send(cmt_msg_t* msg);
static void _handle_morse_decode_flush(cmt_msg_t* msg);
static void _handle_morse_to_decode(cmt_msg_t* msg);
static void _handle_send_be_status(cmt_msg_t* msg);
static void _handle_ui_initialized(cmt_msg_t* msg);
static void _handle_wire_connect(cmt_msg_t* msg);
static void _handle_wire_connect_toggle(cmt_msg_t* msg);
static void _handle_wire_disconnect(cmt_msg_t* msg);
static void _handle_wire_set(cmt_msg_t* msg);

// Idle functions...
static void _be_idle_function_1();
static void _be_idle_function_2();
static void _be_idle_function_3();

static cmt_msg_t _msg_be_send_status;
static cmt_msg_t _msg_be_initialized;

static be_idle_fn_data_t _msg_activity_data = { 0, 0 };

static uint32_t _last_rtc_update_ts; // ms timestamp of the last time we updated the RTC
static uint32_t _last_status_update_ts; // ms timestamp of last status update

#define LEAVE_IDLE_FUNCTION()    {_msg_activity_data.idle_num++; _msg_activity_data.msg_burst = 0;}
#define LEAVE_MSG_HANDLER()    {_msg_activity_data.idle_num = 0; _msg_activity_data.msg_burst++;}

static const msg_handler_entry_t _be_test = { MSG_BE_TEST, _handle_be_test };
static const msg_handler_entry_t _config_changed_handler_entry = { MSG_CONFIG_CHANGED, _handle_config_changed };
static const msg_handler_entry_t _cmt_sm_tick_handler_entry = { MSG_CMT_SLEEP, _handle_cmt_sleep };
static const msg_handler_entry_t _kob_key_read_handler_entry = { MSG_KOB_KEY_READ, _handle_kob_key_read };
static const msg_handler_entry_t _kob_sound_code_cont_handler_entry = { MSG_KOB_SOUND_CODE_CONT, _handle_kob_sound_code_cont };
static const msg_handler_entry_t _mks_keep_alive_send_handler_entry = { MSG_MKS_KEEP_ALIVE_SEND, _handle_mks_keep_alive_send };
static const msg_handler_entry_t _morse_decode_flush_handler_entry = { MSG_MORSE_DECODE_FLUSH, _handle_morse_decode_flush };
static const msg_handler_entry_t _morse_to_decode_handler_entry = { MSG_MORSE_CODE_SEQUENCE, _handle_morse_to_decode };
static const msg_handler_entry_t _send_be_status_handler_entry = { MSG_SEND_BE_STATUS, _handle_send_be_status };
static const msg_handler_entry_t _ui_initialized_handler_entry = { MSG_UI_INITIALIZED, _handle_ui_initialized };
static const msg_handler_entry_t _wire_connect_handler_entry = { MSG_WIRE_CONNECT, _handle_wire_connect };
static const msg_handler_entry_t _wire_connect_toggle_handler_entry = { MSG_WIRE_CONNECT_TOGGLE, _handle_wire_connect_toggle };
static const msg_handler_entry_t _wire_disconnect_handler_entry = { MSG_WIRE_DISCONNECT, _handle_wire_disconnect };
static const msg_handler_entry_t _wire_set_handler_entry = { MSG_WIRE_SET, _handle_wire_set };

// For performance - put these in order that we expect to receive more often
static const msg_handler_entry_t* _be_handler_entries[] = {
    & _cmt_sm_tick_handler_entry,
    & _morse_to_decode_handler_entry,
    & _morse_decode_flush_handler_entry,
    & _kob_key_read_handler_entry,
    & _kob_sound_code_cont_handler_entry,
    & _send_be_status_handler_entry,
    & _mks_keep_alive_send_handler_entry,
    & _wire_connect_handler_entry,
    & _wire_connect_toggle_handler_entry,
    & _wire_disconnect_handler_entry,
    & _wire_set_handler_entry,
    & _config_changed_handler_entry,
    & _ui_initialized_handler_entry,
    & _be_test,
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
//
// Something to do when there are no messages to process.
// (These are cycled through, so do one task.)
// ====================================================================

static void _be_idle_function_1() {
    options_read();  // Re-read the option switches
    LEAVE_IDLE_FUNCTION();
}

static void _be_idle_function_2() {
    uint32_t now = now_ms();
    if (_last_rtc_update_ts + HOUR_IN_MS < now) {
        // Keep the RTC set correctly by making a NTP call.
        _last_rtc_update_ts = now;
        const config_sys_t* cfgsys = config_sys();
        network_update_rtc(cfgsys->tz_offset);
    }
    LEAVE_IDLE_FUNCTION();
}

static void _be_idle_function_3() {
    uint32_t now = now_ms();
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

static void _handle_be_test(cmt_msg_t* msg) {
    // Test `scheduled_msg_ms` error
    static int times = 1;
    static cmt_msg_t msg_time = { MSG_BE_TEST };
    static uint64_t first_t = 0;

    uint64_t period = 60;

    if (mk_debug()) {
        uint64_t now = now_us();
        if (first_t == 0) { first_t = now; }

        uint64_t last_time = msg->data.ts_us;
        int64_t error = ((now - last_time) - (period * 1000 * 1000));
        int64_t total_error = (now - (first_t + (times * (period * 1000 * 1000))));
        float error_per_ms = ((error * 1.0) / (period * 1000.0));
        info_printf("\n%5d - Error us/ms:%5.2f  Avg:%5d\n", times, error_per_ms, (total_error / (times * period)));
    }
    msg_time.data.ts_us = now_us(); // Get the 'next' -> 'last_time' fresh
    schedule_msg_in_ms((period * 1000), &msg_time);
    times++;
}

static void _handle_cmt_sleep(cmt_msg_t* msg) {
    cmt_handle_sleep(msg);
    LEAVE_MSG_HANDLER();
}

static void _handle_config_changed(cmt_msg_t* msg) {
    // Update things that depend on the current configuration.
    const config_t* cfg = config_current();
    // See if things we care about have changed
    if (cfg->wire != _last_cfg->wire) {
        _msg_wire_set.data.wire = cfg->wire;
        postBothMsgBlocking(&_msg_wire_set);
    }
    if (cfg->text_speed != _last_cfg->text_speed
     || cfg->char_speed_min != _last_cfg->char_speed_min
     || cfg->code_type != _last_cfg->code_type
     || cfg->spacing != _last_cfg->spacing) {
        morse_module_init(cfg->text_speed, cfg->char_speed_min, cfg->code_type, cfg->spacing);
    }
    _last_cfg = config_copy(_last_cfg, cfg);
    LEAVE_MSG_HANDLER();
}

static void _handle_kob_key_read(cmt_msg_t* msg) {
    kob_read_code_from_key(msg);
    LEAVE_MSG_HANDLER();
}

static void _handle_kob_sound_code_cont(cmt_msg_t* msg) {
    kob_sound_code_continue();
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
    kob_sound_code(mcode_seq);
    morse_decode(mcode_seq);
    mcode_seq_free(mcode_seq);
    LEAVE_MSG_HANDLER();
}

static void _handle_send_be_status(cmt_msg_t* msg) {
    // Update our status
    LEAVE_MSG_HANDLER();
}

static void _handle_ui_initialized(cmt_msg_t* msg) {
    // The UI has reported that it is initialized.
    // Since we are responding to a message, it means we
    // are also initialized, so -
    //
    // Start things running.

    // Kick off our key code reading
    msg->id = MSG_KOB_KEY_READ;
    msg->data.key_read_state.phase = KEY_READ_START;
    kob_read_code_from_key(msg);

    // Autoconnect?
    if (config_current()->auto_connect) {
        mkwire_connect(config_current()->wire);
    }
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

void be_module_init() {
    _last_rtc_update_ts = 0;
    char hostname[NET_URL_MAX_LEN + 1];
    const config_t* cfg = config_current();
    _last_cfg = config_new(cfg);
    const char* host_and_port = cfg->host_and_port;
    uint16_t port = port_from_hostport(host_and_port, MKOBSERVER_PORT_DEFAULT);
    if (0 == host_from_hostport(hostname, NET_URL_MAX_LEN, host_and_port)) {
        mkwire_module_init(MKOBSERVER_DEFAULT, port, cfg->station, cfg->wire);
    }
    else {
        mkwire_module_init(hostname, port, cfg->station, cfg->wire);
    }
    mks_module_init();
    morse_module_init(cfg->text_speed, cfg->char_speed_min, cfg->code_type, cfg->spacing);
    kob_module_init(cfg->invert_key_input, cfg->key_has_closer);

    // Done with the Backend Initialization - Let the UI know.
    _msg_be_initialized.id = MSG_BE_INITIALIZED;
    postUIMsgBlocking(&_msg_be_initialized);
    // Post a TEST to ourself in case we have any tests set up.
    cmt_msg_t msg = { MSG_BE_TEST };
    postBEMsgNoWait(&msg);
}

void start_be() {
    static bool _started = false;
    // Make sure we aren't already started and that we are being called from core-0.
    assert(!_started && 0 == get_core_num());
    _started = true;
    message_loop(&be_msg_loop_cntx);
}