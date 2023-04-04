/**
 * MuKOB Key On Board (key & sounder) functionality.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "kob.h"
#include "config.h"
#include "mkboard.h"
#include "morse.h"
#include "system_defs.h"

#define _KOB_CODESEQ_MAX_LEN 50 // Code sequences can have a maximum of 50 elements

#define _KEY_READ_DEBOUNCE 15    // time to ignore transitions due to contact bounce(ms)
#define _KOB_CODE_SPACE 120 // amount of space to signal end of code sequence(ms)
#define _KOB_CKT_CLOSE 800  // length of mark to signal circuit closure(ms)

static bool _invert_key_input = false;
static bool _key_has_closer = false;

static bool _circuit_closed = false;
static bool _sounder_energized = false;

// Used for letting the UI know that state changed
cmt_msg_t _msg_kob_status;

// Used for getting code from the key
cmt_msg_t _msg_key_mcode;
cmt_msg_t _msg_key_read_code;
static int _codeseq_index = 0;
static int32_t _codeseq[_KOB_CODESEQ_MAX_LEN + 4];
static bool _key_closer_is_open = false;
static bool _key_was_last_closed = false; // 'false' means open
static uint32_t _key_last_read_time;


void _kob_key_read_code_continue(cmt_msg_t* msg) {
    key_read_state_t state = msg->data.key_read_state;
    bool key_closed;
    uint32_t now;
    uint32_t delta_t;

    if (KEY_READ_DEBOUNCE != state.phase) {
        key_closed = kob_key_is_closed();
        now = now_ms();
        delta_t = now - _key_last_read_time;
    }
    else {
        key_closed = _key_was_last_closed;
        now = _key_last_read_time;
        delta_t = state.delta_time;
    }
    if (KEY_READ_DEBOUNCE == state.phase || key_closed != _key_was_last_closed) {
        _key_was_last_closed = key_closed;
        _key_last_read_time = now;
        if (KEY_READ_DEBOUNCE != state.phase) {
            // Pause for debounce
            _msg_key_read_code.data.key_read_state.phase = KEY_READ_DEBOUNCE;
            _msg_key_read_code.data.key_read_state.delta_time = delta_t;
            schedule_msg_in_ms(_KEY_READ_DEBOUNCE, &_msg_key_read_code);
            return;
        }
        if (key_closed) {
            _codeseq[_codeseq_index++] = (-delta_t);
        }
        else if (_circuit_closed) {
            _codeseq[_codeseq_index++] = (-delta_t);
            _codeseq[_codeseq_index++] = (MORSE_EXTENDED_MARK_END_INDICATOR); // Circuit/Closer Open
            _circuit_closed = false;
            // Let the UI know that the state changed
            _msg_kob_status.data.kob_status.circuit_closed = _circuit_closed;
            _msg_kob_status.data.kob_status.key_closed = _circuit_closed; // Same at this point
            _msg_kob_status.data.kob_status.sounder_energized = _sounder_energized;
            postUIMsgNoWait(&_msg_kob_status);
            // Done assempling this code sequence
            _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
            postBEMsgBlocking(&_msg_key_read_code);
            return;
        }
        else {
            _codeseq[_codeseq_index++] = (delta_t);
        }
    }
    if (!key_closed && _codeseq_index > 0 && now > (_key_last_read_time + _KOB_CODE_SPACE)) {
        // Done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    if (key_closed && !_circuit_closed && now > _key_last_read_time + _KOB_CKT_CLOSE) {
        _codeseq[_codeseq_index++] = (MORSE_EXTENDED_MARK_START_INDICATOR); // Circuit/Closer Closed
        _circuit_closed = true;
        // Let the UI know the closer state changed
        _msg_kob_status.data.kob_status.circuit_closed = _circuit_closed;
        _msg_kob_status.data.kob_status.key_closed = _circuit_closed; // Same at this point
        _msg_kob_status.data.kob_status.sounder_energized = _sounder_energized;
        postUIMsgNoWait(&_msg_kob_status);
        // Done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    if (_codeseq_index >= _KOB_CODESEQ_MAX_LEN) {
        // Max length reached, so done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    // Post a message to ourself. Allows other messages to be processed if queued.
    _msg_key_read_code.data.key_read_state.phase = KEY_READ_CONTINUE;
    _msg_key_read_code.data.key_read_state.delta_time = delta_t;
    postBEMsgBlocking(&_msg_key_read_code);
    return;
}

void kob_init(bool invert_key_input, bool key_has_closer) {
    _invert_key_input = invert_key_input;
    _key_has_closer = key_has_closer;
    _key_closer_is_open = false; // Assume the key closer is starting out closed
    _key_was_last_closed = false; // Set key open to start
    _key_last_read_time = 0;
    _codeseq_index = 0;
    kob_sounder_energize(true);
    _circuit_closed = kob_key_is_closed();
    // Initialize our messages
    _msg_key_read_code.id = MSG_KEY_READ;
    _msg_key_read_code.data.key_read_state.phase = KEY_READ_START;
    // Let the UI know the current status
    _msg_kob_status.id = MSG_KOB_STATUS;
    _msg_kob_status.data.kob_status.circuit_closed = _circuit_closed;
    _msg_kob_status.data.kob_status.key_closed = _circuit_closed; // Same at this point
    _msg_kob_status.data.kob_status.sounder_energized = _sounder_energized;
    postUIMsgNoWait(&_msg_kob_status);
}

extern bool kob_key_is_closed(void) {
    bool closed = (KOB_KEY_CLOSED == gpio_get(KOB_KEY_IN));

    return (_invert_key_input ? !closed : closed);
}

/**
 * Message handler to start/continue/finish reading code from the key.
 */
void kob_read_code_from_key(cmt_msg_t* msg) {
    // If our message is also in the scheduled messages, cancel it.
    scheduled_msg_cancel(scheduled_message_get(&_msg_key_read_code));
    if (KEY_READ_COMPLETE == msg->data.key_read_state.phase) {
        // Check the assembled code
        if (_codeseq_index > 0) {
            if (MORSE_EXTENDED_MARK_START_INDICATOR == _codeseq[_codeseq_index - 1]) {
                _key_closer_is_open = false;
            }
            else if (MORSE_EXTENDED_MARK_END_INDICATOR == _codeseq[_codeseq_index - 1]) {
                _key_closer_is_open = true;
            }
            // Send it off to be handled
            mcode_seq_t* mcode_seq = mcode_seq_alloc(MCODE_SRC_KEY, _codeseq, _codeseq_index);
            _msg_key_mcode.id = MSG_MORSE_CODE_SEQUENCE;
            _msg_key_mcode.data.mcode_seq = mcode_seq;
            postBEMsgBlocking(&_msg_key_mcode);
        }
        _codeseq_index = 0;
        // Post a message to read again
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_START;
        _msg_key_read_code.data.key_read_state.delta_time = 0;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    else if (KEY_READ_START == msg->data.key_read_state.phase) {
        _codeseq_index = 0;
    }
    _kob_key_read_code_continue(msg);
    return;
}

void kob_sounder_energize(bool energize) {
    gpio_put(KOB_SOUNDER_OUT, (energize ? KOB_SOUNDER_ENERGIZED : KOB_SOUNDER_DEENERGIZED));
    _sounder_energized = energize;
}
