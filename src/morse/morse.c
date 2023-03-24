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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static code_type_t _code_type;
static uint8_t _cwpm_min;
static code_spacing_t _spacing;
static uint8_t _twpm;

static int32_t _char_space; // Time between characters (ms)
static int32_t _dash_len;
static int32_t _dot_len; // Length of a dot (ms) at the character speed
static int32_t _space; // Delay before next code element (ms)
static int32_t _word_space; // Time between words (ms)

mcode_t* mcode_alloc(int32_t* codelist, int len) {
    mcode_t* mcode = (mcode_t*)malloc(sizeof(mcode_t));
    mcode->len = len;
    mcode->code = (int32_t*)malloc(len * sizeof(int32_t));
    memcpy(mcode->code, codelist, len * sizeof(int32_t));

    return (mcode);
}

void mcode_free(mcode_t* mcode){
    if (mcode) {
        free (mcode->code);
        free (mcode);
    }
}

mcode_t* morse_encode(char c) {
    int32_t codelist[20]; // Large enough for 2 values per element - longest char is 9 elements (',-)
    int cli = 0; // Code list index. Used while building.
    char** code_table = (CODE_TYPE_AMERICAN == _code_type ? american_morse : international_morse);

    c = toupper(c);
    // Check for characters not in table
    if (c < SP || c > 'Z') {
        switch (c) {
            case '\r':
            case '\n':
                break;
            case '~':
                codelist[cli++] = (-_space);
                codelist[cli++] = (2);
                break;
            default:
                _space += (_word_space - _char_space);
                break;
        }
    }
    else {
        // Process the elements for the character (from the table)
        int cti = (c - SP);
        char* elements = code_table[cti];
        char element;
        while ('\000' != (element = *elements++)) {
            if (SP == element) {
                _space = (3 * _dot_len);
            }
            else {
                codelist[cli++] = (-_space);
                switch (element) {
                    case '.':
                        codelist[cli++] = _dot_len;
                        break;
                    case '-':
                        codelist[cli++] = _dash_len;
                        break;
                    case '=':
                        codelist[cli++] = (2 * _dash_len);
                        break;
                    case '#': // Not in the table, but in the morse.py code
                        codelist[cli++] = (9 * _dot_len);
                        break;
                    default: // Handles '`' in our table. This is for characters that don't have Morse.
                        _space += (_word_space - _char_space);
                        break;
                }
                _space = _dot_len;
            }
        }
        _space = _char_space;
    }
    // Allocate the codelist structure to return
    mcode_t* mcode = mcode_alloc(codelist, cli);

    return (mcode);
}

void morse_init(uint8_t twpm, uint8_t cwpm_min, code_type_t code_type, code_spacing_t spacing) {
    _code_type = code_type;
    _spacing = spacing;
    _twpm = twpm;
    if (CODE_SPACING_NONE == spacing) {
        _cwpm_min = twpm; // Send characters at the overall text speed
    }
    else {
        _cwpm_min = (cwpm_min > twpm ? cwpm_min : twpm); // Larger of the two for Farnsworth timing
    }
    _dot_len = (UNIT_DOT_TIME / _cwpm_min); // Lenth of a dot (in ms) for this character speed
    _char_space = (3 * _dot_len);
    _word_space = (7 * _dot_len);
    if (CODE_TYPE_AMERICAN == _code_type) {
        // Adjustments for American code
        _char_space += ((60000 / _cwpm_min - _dot_len * DOTS_PER_WORD) / 6);
        _word_space = (2 * _char_space);
    }
    if (CODE_SPACING_NONE != _spacing) {
        double delta = ((60000.0 / _twpm) - (60000.0 / _cwpm_min)); // Amount to stretch each word
        if (CODE_SPACING_CHAR == _spacing) {
            _char_space += (int32_t)(delta / 6);
            _word_space += (int32_t)(delta / 3);
        }
        if (CODE_SPACING_WORD == _spacing) {
            _word_space += (int32_t)delta;
        }
    }
    _dash_len = (3 * _dot_len);
    _space = _word_space; // Delay before next code element (ms)
}