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
#include "mks.h"
#include "morse.h"
#include "system_defs.h"

#include <stdlib.h>


#define _KEY_READ_DEBOUNCE 15    // time to ignore transitions due to contact bounce(ms)
#define _KOB_CODE_SENDER_CHG_BREAK -3000 // Long pause, Sender change, break in sequence
#define _KOB_CODE_SPACE 120 // amount of space to signal end of code sequence(ms)
#define _KOB_CKT_CLOSE 800  // length of mark to signal circuit closure(ms)

static bool _invert_key_input = false;
static bool _key_has_closer = false;

static kob_status_t _kob_status;

// Used for letting the UI know that state changed
static cmt_msg_t _msg_kob_status;

// Used for getting code from the key
static cmt_msg_t _msg_key_mcode;
static cmt_msg_t _msg_key_read_code;
static int _kr_codeseq_index = 0;
static int32_t _kr_codeseq[MKS_CODESEQ_MAX_LEN + 4];
static bool _key_closer_is_open = false;
static bool _key_was_last_closed = false; // 'false' means open
static uint32_t _key_last_read_time;

// Used for sounding code
static cmt_msg_t _msg_sound_code;
static mcode_seq_t* _snd_mcode_seq;
static int _snd_code_index;
static bool _snd_phase1;
static uint32_t _snd_t_last;

void _kob_key_read_code_continue(cmt_msg_t* msg) {
    key_read_state_t state = msg->data.key_read_state;
    uint32_t now;
    uint32_t delta_t;

    if (KEY_READ_DEBOUNCE != state.phase) {
        _kob_status.key_closed = kob_key_is_closed();
        now = now_ms();
        delta_t = now - _key_last_read_time;
    }
    else {
        _kob_status.key_closed = _key_was_last_closed;
        now = _key_last_read_time;
        delta_t = state.delta_time;
    }
    if (KEY_READ_DEBOUNCE == state.phase || _kob_status.key_closed != _key_was_last_closed) {
        _key_was_last_closed = _kob_status.key_closed;
        _key_last_read_time = now;
        if (KEY_READ_DEBOUNCE != state.phase) {
            // Pause for debounce
            _msg_key_read_code.data.key_read_state.phase = KEY_READ_DEBOUNCE;
            _msg_key_read_code.data.key_read_state.delta_time = delta_t;
            schedule_msg_in_ms(_KEY_READ_DEBOUNCE, &_msg_key_read_code);
            return;
        }
        if (_kob_status.key_closed) {
            _kr_codeseq[_kr_codeseq_index++] = (-delta_t);
        }
        else if (_kob_status.circuit_closed) {
            _kr_codeseq[_kr_codeseq_index++] = (-delta_t);
            _kr_codeseq[_kr_codeseq_index++] = (MORSE_EXTENDED_MARK_END_INDICATOR); // Circuit/Closer Open
            _kob_status.circuit_closed = false;
            // Let the UI know that the state changed
            _msg_kob_status.data.kob_status.circuit_closed = _kob_status.circuit_closed;
            _msg_kob_status.data.kob_status.key_closed = _kob_status.key_closed;
            _msg_kob_status.data.kob_status.sounder_energized = _kob_status.sounder_energized;
            postUIMsgNoWait(&_msg_kob_status);
            // Done assempling this code sequence
            _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
            postBEMsgBlocking(&_msg_key_read_code);
            return;
        }
        else {
            _kr_codeseq[_kr_codeseq_index++] = (delta_t);
        }
    }
    if (!_kob_status.key_closed && _kr_codeseq_index > 0 && now > (_key_last_read_time + _KOB_CODE_SPACE)) {
        // Done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    if (_kob_status.key_closed && !_kob_status.circuit_closed && now > _key_last_read_time + _KOB_CKT_CLOSE) {
        _kr_codeseq[_kr_codeseq_index++] = (MORSE_EXTENDED_MARK_START_INDICATOR); // Circuit/Closer Closed
        _kob_status.circuit_closed = true;
        // Let the UI know the closer state changed
        _msg_kob_status.data.kob_status.circuit_closed = _kob_status.circuit_closed;
        _msg_kob_status.data.kob_status.key_closed = _kob_status.key_closed;
        _msg_kob_status.data.kob_status.sounder_energized = _kob_status.sounder_energized;
        postUIMsgNoWait(&_msg_kob_status);
        // Done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    if (_kr_codeseq_index >= MKS_CODESEQ_MAX_LEN) {
        // Max length reached, so done assempling this code sequence
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_COMPLETE;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    // Post a message to ourself. Allows other messages to be processed if queued.
    _msg_key_read_code.data.key_read_state.phase = KEY_READ_CONTINUE;
    _msg_key_read_code.data.key_read_state.delta_time = delta_t;
    schedule_msg_in_ms(1, &_msg_key_read_code);
    return;
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
    scheduled_msg_cancel(MSG_KOB_KEY_READ);
    if (KEY_READ_COMPLETE == msg->data.key_read_state.phase) {
        // Check the assembled code
        if (_kr_codeseq_index > 0) {
            if (MORSE_EXTENDED_MARK_START_INDICATOR == _kr_codeseq[_kr_codeseq_index - 1]) {
                _key_closer_is_open = false;
            }
            else if (MORSE_EXTENDED_MARK_END_INDICATOR == _kr_codeseq[_kr_codeseq_index - 1]) {
                _key_closer_is_open = true;
            }
            // Send it off to be handled
            mcode_seq_t* mcode_seq = mcode_seq_alloc(MCODE_SRC_KEY, _kr_codeseq, _kr_codeseq_index);
            _msg_key_mcode.id = MSG_MORSE_CODE_SEQUENCE;
            _msg_key_mcode.data.mcode_seq = mcode_seq;
            postBEMsgBlocking(&_msg_key_mcode);
        }
        _kr_codeseq_index = 0;
        // Post a message to read again
        _msg_key_read_code.data.key_read_state.phase = KEY_READ_START;
        _msg_key_read_code.data.key_read_state.delta_time = 0;
        postBEMsgBlocking(&_msg_key_read_code);
        return;
    }
    else if (KEY_READ_START == msg->data.key_read_state.phase) {
        _kr_codeseq_index = 0;
    }
    _kob_key_read_code_continue(msg);
    return;
}

void kob_sound_code_continue() {
    static int32_t c;
    static int32_t dt = 0;
    static uint32_t t;
    static uint32_t t_next;

    // Process the code
    while (_snd_mcode_seq && _snd_code_index < _snd_mcode_seq->len) {
        if (_snd_phase1) {
            t = now_ms();
            c = _snd_mcode_seq->code_seq[_snd_code_index];
            if (c < _KOB_CODE_SENDER_CHG_BREAK) {
                c = -1; // Adjust to just de-energize sounder
            }
            if (MORSE_EXTENDED_MARK_START_INDICATOR == c || c > MORSE_EXTENDED_MARK_END_INDICATOR) {
                kob_sounder_energize(true);
            }
            t_next = _snd_t_last + abs(c);
            dt = t_next - t;
            if (dt <= 0) {
                _snd_t_last = t;
            }
            else {
                _snd_t_last = t_next;
                // sleep
                _snd_phase1 = false;
                dt = (dt < 5000 ? dt : 5000); // Safeguard sleep time.
                schedule_msg_in_ms(dt, &_msg_sound_code);
                return;
            }
        }
        if (c > 1) { // End of non-latching mark
            kob_sounder_energize(false);
        }
        _snd_code_index++;
        _snd_phase1 = true;
    }

    mcode_seq_free(_snd_mcode_seq);
    _snd_mcode_seq = NULL;
}

void kob_sound_code(mcode_seq_t* mcode_seq) {
    scheduled_msg_cancel(MSG_KOB_SOUND_CODE_CONT);
    mcode_seq_free(_snd_mcode_seq);
    // See if we are suppose to sound this and have an output device enabled
    const config_t* cfg = config_current();
    if (cfg->sound || cfg->sounder) {
        _snd_mcode_seq = mcode_seq_copy(mcode_seq);
        _snd_code_index = 0;
        _snd_phase1 = true;
        kob_sound_code_continue();
    }
}

void kob_sounder_energize(bool energize) {
    gpio_put(KOB_SOUNDER_OUT, (energize ? KOB_SOUNDER_ENERGIZED : KOB_SOUNDER_DEENERGIZED));
    _kob_status.sounder_energized = energize;
}

const kob_status_t* kob_status() {
    return (&_kob_status);
}

void kob_module_init(bool invert_key_input, bool key_has_closer) {
    _invert_key_input = invert_key_input;
    _key_has_closer = key_has_closer;
    _key_closer_is_open = false; // Assume the key closer is starting out closed
    _key_was_last_closed = false; // Set key open to start
    _key_last_read_time = 0;
    _kr_codeseq_index = 0;
    _snd_t_last = now_ms();
    kob_sounder_energize(true);
    _kob_status.circuit_closed = kob_key_is_closed();
    _kob_status.key_closed = kob_key_is_closed();
    // Initialize our messages
    _msg_key_read_code.id = MSG_KOB_KEY_READ;
    _msg_key_read_code.data.key_read_state.phase = KEY_READ_START;
    _msg_sound_code.id = MSG_KOB_SOUND_CODE_CONT;
    // Let the UI know the current status
    _msg_kob_status.id = MSG_KOB_STATUS;
    _msg_kob_status.data.kob_status.circuit_closed = _kob_status.circuit_closed;
    _msg_kob_status.data.kob_status.key_closed = _kob_status.circuit_closed;
    _msg_kob_status.data.kob_status.sounder_energized = _kob_status.sounder_energized;
    postUIMsgNoWait(&_msg_kob_status);
}
