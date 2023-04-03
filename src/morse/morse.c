/**
 * MuKOB Morse (encode & decode) functionality.
 *
 * The operation of this is based on the morse.py module in PyKOB
 * @see https://github.com/MorseKOB/PyKOB
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "morse.h"
#include "morse_tables.h" // Morse data that is in text files in PyKOB
#include "cmt.h"
#include "debugging.h"
#include "util.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Internal function declarations

static char _d_lookup_char(char* dds_buf);
static void _d_post_decoded_text(char* cs, float spacing);
static void _mstr_clear(char* dds_buf);

// Class data

static code_type_t _code_type;

// For 'DECODE' we use a few static string buffers, rather than calloc/free over and over.
#define _MSTRING_ALLOC_SIZE 32
static char _d_mstr_1[_MSTRING_ALLOC_SIZE];
static char _d_mstr_2[_MSTRING_ALLOC_SIZE];
static char _d_mstr_3[_MSTRING_ALLOC_SIZE];
static char _d_mstr_4[_MSTRING_ALLOC_SIZE];

/**
 * @brief Decode morse (code duration) sequence processing data
 * @ingroup morse
 *
 * This holds data to build morse elements into based on timings
 * to allow them to be converted to a character (looked up from a code table).
 */
typedef struct _DECODE_PROC_DATA_ {
    char* morse_elements; // String buffer to build the dots-dashes into
    float space_before;   // space before each character
    float mark_len;       // length of last dot or dash in character
} decode_proc_data_t;

#define _D_CHAR_ONE 0
#define _D_CHAR_TWO 1
#define _D_BOTH_CHARS 2
// 'DECODE' data
static decode_proc_data_t _d_process[_D_BOTH_CHARS]; // Processing data to build two Morse elements
static bool _d_circuit_latched_closed; // True if cicuit has been latched closed by a +1 code element
static int16_t _d_complete_chars; // number of complete characters in buffer
static float _d_mark_len_total; // accumulates the length of a mark as positive code elements are received
static float _d_space_len_total; // accumulates the length of a space as negative code elements are received
// One-time calculated values based on the configured code speed
static float _d_dot_len; // nominal dot length (ms)
static float _d_tru_dot; // actual length of typical dot(ms)
static uint8_t _d_wpm; // configured code speed (max of text and char speeds)
// Scheduled message to cause character to be 'flushed' if time elapses before two have been received.
static cmt_msg_t _decode_flusher_msg;
static scheduled_msg_id_t _d_flusher_id;
// Detected code speed values. Start with the configured speed and calculated values
static float _d_detected_dot_len; // = _d_dot_len
static float _d_detected_tru_dot; // = _d_truDot
static uint8_t _d_detected_wpm;   // = _d_wpm

// 'ENCODE' data
static code_spacing_t _e_spacing;
static uint8_t _e_cwpm_min;
static uint8_t _e_twpm;
static int32_t _e_char_space; // Time between characters (ms)
static int32_t _e_dash_len;
static int32_t _e_dot_len; // Length of a dot (ms) at the character speed
static int32_t _e_space; // Delay before next code element (ms)
static int32_t _e_word_space; // Time between words (ms)

/**
 * @brief Decode the current character with the next_space.
 * @ingroup morse
 *
 * @param next_space The next space time value
 */
static void _d_decode_char(float next_space) {
    float sp1 = _d_process[ _D_CHAR_ONE ].space_before; // space before 1st character
    float sp2 = _d_process[ _D_CHAR_TWO ].space_before; // space before 2nd character
    float sp3 = next_space; // space before next character
    char* code = _d_mstr_3; // use one of our Morse-String buffers
    char* cs = _d_mstr_4; // use another Morse-String buffers
    _mstr_clear(code);
    _mstr_clear(cs);

    _d_complete_chars += 1; // number of complete characters in buffer (1 or 2)
    if (_d_complete_chars == _D_BOTH_CHARS && sp2 < (MD_MAX_MORSE_SPACE * _d_dot_len)
        && (MD_MORSE_RATIO * sp1) > sp2 && sp2 < (MD_MORSE_RATIO * sp3)) {
        // could be two halves of a spaced character
        // try combining the two halves
        strcat(code, _d_process[ _D_CHAR_ONE ].morse_elements); strcat(code, " "); strcat(code, _d_process[ _D_CHAR_TWO ].morse_elements);
        *cs = _d_lookup_char(code);
        if (*cs != '\000' && *cs != '&') {
            // yes, it's a spaced character, clear the buffers
            _d_process[ _D_CHAR_TWO ].space_before = 0.0;
            _mstr_clear(_d_process[ _D_CHAR_ONE ].morse_elements);
            _d_process[ _D_CHAR_ONE ].mark_len = 0.0;
            _mstr_clear(_d_process[ _D_CHAR_TWO ].morse_elements);
            _d_process[ _D_CHAR_TWO ].mark_len = 0.0;
            _d_complete_chars = 0;
        }
        else {
            // it's not recognized as a spaced character,
            _mstr_clear(code);
            *cs = '\000';
        }
    }
    if (_d_complete_chars == _D_BOTH_CHARS && sp2 < (MD_MIN_CHAR_SPACE * _d_dot_len)) {
        // it's a single character, merge the two halves
        strcat(_d_process[ _D_CHAR_ONE ].morse_elements, _d_process[ _D_CHAR_TWO ].morse_elements);
        _d_process[ _D_CHAR_ONE ].mark_len = _d_process[ _D_CHAR_TWO ].mark_len;
        _mstr_clear(_d_process[ _D_CHAR_TWO ].morse_elements);
        _d_process[ _D_CHAR_TWO ].space_before = 0.0;
        _d_process[ _D_CHAR_TWO ].mark_len = 0.0;
        _d_complete_chars = 1;
    }
    if (_d_complete_chars == _D_BOTH_CHARS) {
        // decode the first character, otherwise wait for the next one to arrive
        strcpy(code, _d_process[ _D_CHAR_ONE ].morse_elements);
        *cs = _d_lookup_char(code);
        if (*cs == 'T' && _d_process[ _D_CHAR_ONE ].mark_len > (MD_MAX_DASH_LEN * _d_dot_len)) {
            *cs = '_';
        }
        else if (*cs == 'T' && _d_process[ _D_CHAR_ONE ].mark_len > (MD_MIN_L_LEN * _d_dot_len) &&
                 CODE_TYPE_AMERICAN == _code_type) {
            *cs = 'L';
        }
        else if (*cs == 'E') {
            if (_d_process[ _D_CHAR_ONE ].mark_len == 1.0) {
                *cs = '_';
            }
            else if (_d_process[ _D_CHAR_ONE ].mark_len == 2.0) {
                *cs = '_';
                sp1 = 0; // ZZZ eliminate space between underscores
            }
        }
        strcpy(_d_process[ _D_CHAR_ONE ].morse_elements, _d_process[ _D_CHAR_TWO ].morse_elements);
        _d_process[ _D_CHAR_ONE ].space_before = _d_process[ _D_CHAR_TWO ].space_before;
        _d_process[ _D_CHAR_ONE ].mark_len = _d_process[ _D_CHAR_TWO ].mark_len;
        _mstr_clear(_d_process[ _D_CHAR_TWO ].morse_elements);
        _d_process[ _D_CHAR_TWO ].space_before = 0.0;
        _d_process[ _D_CHAR_TWO ].mark_len = 0.0;
        _d_complete_chars = 1;
    }
    _d_process[_d_complete_chars].space_before = next_space;
    float spacing = ((sp1 / (3.0 * _d_tru_dot)) - 1.0);
    if (*code && *cs == '\000') {
        strcpy(cs, "["); strcat(cs, code); strcat(cs, "]");
        _d_post_decoded_text(cs, spacing);
    }
    else if (*cs != '\000') {
        _d_post_decoded_text(cs, spacing);
    }
}

static char _d_lookup_char(char* dds) {
    if (*dds) {
        const char** morse_table = (CODE_TYPE_AMERICAN == _code_type ? american_morse : international_morse);
        for (int i = 0; i < 60; i++) {
            if (strcmp(dds, morse_table[i]) == 0) {
                return (' ' + i);
            }
        }
    }
    return ('\000');
}

static void _d_post_decoded_text(char* cs, float spacing) {
    // Generate a string with leading spacing and the character.
    char txt[32];
    memset(txt, '\000', sizeof(txt));

    if (CODE_TYPE_AMERICAN == _code_type) {
        spacing = (spacing - 0.25) / 1.25; // adjust for American Morse spacing
    }
    if (spacing > 100.0) {
        strcpy(txt, ('_' == *cs ? "" : " * "));
    }
    else if (spacing > 5.0) {
        spacing = 5.0;
    }
    if (spacing <= 5.0) {
        int sps = (int)(spacing + 0.5);
        char* t = txt;
        for (int i = 0; i < sps; i++) {
            *t++ = ' ';
        }
        *t = '\000';
    }
    // Append the char
    strcat(txt, cs);
    // Post the string
    cmt_msg_t msg;
    msg.id = MSG_CODE_TEXT;
    msg.data.str = str_value_create(txt);
    postUIMsgBlocking(&msg);
}

static void _d_update_detected_wpm(mcode_seq_t* mcode_seq) {
    // @todo: Fill this code in to calculate the actual (perceved) speed.
}

/**
 * @brief Fill an Morse-String buffer with Nulls.
 * @ingroup morse
 *
 * @param mstr_buf A character buffer that is `_MSTRING_ALLOC_SIZE` long.
 */
static void _mstr_clear(char* mstr_buf) {
    memset(mstr_buf, '\000', _MSTRING_ALLOC_SIZE);
}

/**
 * @brief Append a character to the dits & dahs in a DD's buffer.
 * @ingroup morse
 *
 * This will append a character to a DD's buffer. This requires
 * that the DD's buffer started out filled with Nulls, including
 * the additional terminating Null. It accomplishes the 'append' by looking
 * for the first Null within the `_MSTRING_ALLOC_SIZE` limit.
 *
 * @param c The character to append.
 */
static void _mstr_append(char* mstr_buf, char c) {
    for (int i = 0; i < _MSTRING_ALLOC_SIZE; i++) {
        if (!mstr_buf[i]) {
            mstr_buf[i] = c;
            break;
        }
    }
}

mcode_seq_t* mcode_seq_alloc(int32_t* code_seq, int len) {
    mcode_seq_t* mcode_seq = (mcode_seq_t*)malloc(sizeof(mcode_seq_t));
    mcode_seq->len = len;
    mcode_seq->code_seq = (int32_t*)malloc(len * sizeof(int32_t));
    memcpy(mcode_seq->code_seq, code_seq, len * sizeof(int32_t));

    return (mcode_seq);
}

void mcode_seq_free(mcode_seq_t* mcode_seq){
    if (mcode_seq) {
        free (mcode_seq->code_seq);
        free (mcode_seq);
    }
}

/*
    The Morse decoding algorithm has to wait until two characters have been received (or some
    time has passed) before decoding either of them. This is because what appears to be two
    characters may be two halves of a single spaced character. The two characters are kept in
    a buffer which (clumsily) is represented as three lists: code_buf, space_buf, and mark_buf
    (see details in the `init` function).
*/
void morse_decode(mcode_seq_t* mcode_seq) {
    if (debugging_flags & DEBUGGING_MORSE_DECODE_SKIP) {
        return;
    }
    // Code received, so cancel a pending flush timer
    scheduled_msg_cancel(_d_flusher_id);
    _d_flusher_id = SCHED_MSG_ID_INVALID;
    // _d_update_detected_wpm(mcode_seq);
    // Run through the code list
    for (int i = 0; i < mcode_seq->len; i++) {
        int32_t c = mcode_seq->code_seq[i];
        if (c < 0) {
            // start or continuation of space, or continuation of mark (if latched)
            c = (-c);
            if (_d_circuit_latched_closed) {
                // circuit has been latched closed
                _d_mark_len_total += (float)c;
            }
            else if (_d_space_len_total > 0.0) {
                // continuation of space
                _d_space_len_total += (float)c;
            }
            else {
                // end of mark
                if (_d_mark_len_total > (MD_MIN_DASH_LEN * _d_tru_dot)) {
                    _mstr_append(_d_process[_d_complete_chars].morse_elements, '-'); // dash
                }
                else {
                    _mstr_append(_d_process[_d_complete_chars].morse_elements, '.'); // dot
                }
                _d_process[_d_complete_chars].mark_len = _d_mark_len_total;
                _d_mark_len_total = 0.0;
                _d_space_len_total = (float)c;
            }
        }
        else if (c == MORSE_EXT_MARK_START_INDICATOR) {
            // start(or continuation) of extended mark
            _d_circuit_latched_closed = true;
            if (_d_space_len_total > 0.0) {
                // start of mark
                if (_d_space_len_total > (MD_MIN_MORSE_SPACE * _d_dot_len)) {
                    // possible Morse or word space
                    _d_decode_char(_d_space_len_total);
                    _d_mark_len_total = 0.0;
                    _d_space_len_total = 0.0;
                }
                else {
                    // continuation of mark
                }
            }
        }
        else if (c == MORSE_MARK_END_INDICATOR) {
            // end of mark (or continuation of space)
            _d_circuit_latched_closed = false;
        }
        else if (c > 2) {
            // mark
            _d_circuit_latched_closed = false;
            if (_d_space_len_total > 0.0) {
                // start of new mark
                if (_d_space_len_total > (MD_MIN_MORSE_SPACE * _d_dot_len)) {
                    // possible Morse or word space
                    _d_decode_char(_d_space_len_total);
                }
                _d_mark_len_total = (float)c;
                _d_space_len_total = 0.0;
            }
            else if (_d_mark_len_total > 0.0) {
                // continuation of mark
                _d_mark_len_total += (float)c;
            }
        }
    }
    // Set up a 'flusher' alarm (skip if debugging decode)
    if (!(debugging_flags & DEBUGGING_MORSE_DECODE)) {
        _d_flusher_id = schedule_msg_in_ms((20 * _d_tru_dot), &_decode_flusher_msg);
    }
}

void morse_decode_flush() {
    if (_d_mark_len_total > 0 || _d_circuit_latched_closed) {
        float spacing = _d_process[_d_complete_chars].space_before;
        if (_d_mark_len_total > (MD_MIN_DASH_LEN * _d_tru_dot)) {
            _mstr_append(_d_process[_d_complete_chars].morse_elements, '-'); // dash
        }
        else if (_d_mark_len_total > 2.0) {
            _mstr_append(_d_process[_d_complete_chars].morse_elements, '.'); // dot
        }
        _d_process[_d_complete_chars].mark_len = _d_mark_len_total;
        _d_mark_len_total = 0;
        _d_space_len_total = 1; // to prevent circuit opening mistakenly decoding as 'E'
        _d_decode_char(MORSE_CODE_ELEMENT_VALUE_MAX);
        _d_decode_char(MORSE_CODE_ELEMENT_VALUE_MAX); // a second time, to flush both characters
        _mstr_clear(_d_process[ _D_CHAR_ONE ].morse_elements);
        _d_process[ _D_CHAR_ONE ].space_before = 0.0;
        _d_process[ _D_CHAR_ONE ].mark_len = 0.0;
        _mstr_clear(_d_process[ _D_CHAR_TWO ].morse_elements);
        _d_process[ _D_CHAR_TWO ].space_before = 0.0;
        _d_process[ _D_CHAR_TWO ].mark_len = 0.0;
        _d_complete_chars = 0;
        if (_d_circuit_latched_closed) {
            _d_post_decoded_text("_", ((spacing / (3.0 * _d_tru_dot)) - 1.0));
        }
    }
}

mcode_seq_t* morse_encode(char c) {
    int32_t code_seq[(2 * MORSE_MAX_DDS_IN_CHAR) + 1]; // Enough for 2 ints per element - longest is 9 elements (',-)
    int cli = 0; // Code sequence index. Used while building.
    const char** code_table = (CODE_TYPE_AMERICAN == _code_type ? american_morse : international_morse);

    c = toupper(c);
    // Check for characters not in table
    if (c < SP || c > 'Z') {
        switch (c) {
            case '\r':
            case '\n':
                break;
            case '~':
                code_seq[cli++] = (-_e_space);
                code_seq[cli++] = MORSE_MARK_END_INDICATOR;
                break;
            default:
                _e_space += (_e_word_space - _e_char_space);
                break;
        }
    }
    else {
        // Process the elements for the character (from the table)
        int cti = (c - SP);
        const char* elements = code_table[cti];
        char element;
        while ('\000' != (element = *elements++)) {
            if (SP == element) {
                _e_space = (3 * _e_dot_len);
            }
            else {
                code_seq[cli++] = (-_e_space);
                switch (element) {
                    case '.':
                        code_seq[cli++] = _e_dot_len;
                        break;
                    case '-':
                        code_seq[cli++] = _e_dash_len;
                        break;
                    case '=': // 'L' (long dash)
                        code_seq[cli++] = (2 * _e_dash_len);
                        break;
                    case '~': // '0' (extra long dash)
                        code_seq[cli++] = (3 * _e_dash_len);
                        break;
                    case '#': // Not in the table, but in the morse.py code
                        code_seq[cli++] = (9 * _e_dot_len);
                        break;
                    default: // Handles '`' in our table. This is for characters that don't have Morse.
                        _e_space += (_e_word_space - _e_char_space);
                        break;
                }
                _e_space = _e_dot_len;
            }
        }
        _e_space = _e_char_space;
    }
    // Allocate the codelist structure to return
    mcode_seq_t* mcode_seq = mcode_seq_alloc(code_seq, cli);

    return (mcode_seq);
}

void morse_init(uint8_t twpm, uint8_t cwpm_min, code_type_t code_type, code_spacing_t spacing) {
    _code_type = code_type;

    // Decode values
    _d_wpm = (twpm > cwpm_min ? twpm : cwpm_min);
    _d_dot_len = (UNIT_DOT_TIME / _d_wpm);
    _d_tru_dot = _d_dot_len;
    _d_flusher_id = SCHED_MSG_ID_INVALID;
    _d_complete_chars = 0;
    _d_circuit_latched_closed = false;
    _d_process[ _D_CHAR_ONE ].morse_elements = _d_mstr_1;
    _d_process[ _D_CHAR_TWO ].morse_elements = _d_mstr_2;
    _mstr_clear(_d_process[ _D_CHAR_ONE ].morse_elements);
    _mstr_clear(_d_process[ _D_CHAR_TWO ].morse_elements);
    _d_mark_len_total = 0.0;
    _d_space_len_total = 1.0;
    _d_detected_wpm = _d_wpm;
    _d_detected_dot_len = _d_dot_len;
    _d_detected_tru_dot = _d_tru_dot;
    _decode_flusher_msg.id = MSG_MORSE_DECODE_FLUSH;

    // Encode values
    _e_spacing = spacing;
    _e_twpm = twpm;
    if (CODE_SPACING_NONE == spacing) {
        _e_cwpm_min = twpm; // Send characters at the overall text speed
    }
    else {
        _e_cwpm_min = (cwpm_min > twpm ? cwpm_min : twpm); // Larger of the two for Farnsworth timing
    }
    _e_dot_len = (UNIT_DOT_TIME / _e_cwpm_min); // Lenth of a dot (in ms) for this character speed
    _e_char_space = (3 * _e_dot_len);
    _e_word_space = (7 * _e_dot_len);
    if (CODE_TYPE_AMERICAN == _code_type) {
        // Adjustments for American code
        _e_char_space += ((60000 / _e_cwpm_min - _e_dot_len * DOTS_PER_WORD) / 6);
        _e_word_space = (2 * _e_char_space);
    }
    if (CODE_SPACING_NONE != _e_spacing) {
        float delta = ((60000.0 / _e_twpm) - (60000.0 / _e_cwpm_min)); // Amount to stretch each word
        if (CODE_SPACING_CHAR == _e_spacing) {
            _e_char_space += (int32_t)(delta / 6.0);
            _e_word_space += (int32_t)(delta / 3.0);
        }
        if (CODE_SPACING_WORD == _e_spacing) {
            _e_word_space += (int32_t)delta;
        }
    }
    _e_dash_len = (3 * _e_dot_len);
    _e_space = _e_word_space; // Delay before next code element (ms)
}