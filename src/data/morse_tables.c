/**
 * MuKOB Morse conversion tables - American and International
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#include "morse_tables.h"

const char* american_morse[] = {
    "   ",          // SP
    "---.",         // !
    "`",            // "
    "`",            // #
    "... .-..",     // $
    "`",            // %
    ". ...",        // &
    "..-. .-..",    // '
    "`",            // (
    "`",            // )
    "`",            // *
    "+",            // + (Marker: close)
    ".-.-",         // ,
    ".... .-..",    // -
    "..--..",       // .
    "..--",         // /
    "~",            // 0 (actually an extra-long dah, but commonly 'O')
    ".--.",         // 1
    "..-..",        // 2
    "...-.",        // 3
    "....-",        // 4
    "---",          // 5
    "......",       // 6
    "--..",         // 7
    "-....",        // 8
    "-..-",         // 9
    "-.-..",        // :
    "`",            // ;
    "`",            // <
    "----",         // =
    "`",            // >
    "-..-.",        // ?
    "`",            // @
    ".-",           // A
    "-...",         // B
    ".. .",         // C
    "-..",          // D
    ".",            // E
    ".-.",          // F
    "--.",          // G
    "....",         // H
    "..",           // I
    "-.-.",         // J
    "-.-",          // K
    "=",            // L
    "--",           // M
    "-.",           // N
    ". .",          // O
    ".....",        // P
    "..-.",         // Q
    ". ..",         // R
    "...",          // S
    "-",            // T
    "..-",          // U
    "...-",         // V
    ".--",          // W
    ".-..",         // X
    ".. ..",        // Y
    "... .",        // Z
};
int mta_len = (sizeof(american_morse) / sizeof(char*));

const char* international_morse[] = {
    "    ",         // SP
    "-.-.--",       // !
    ".-..-.",       // "
    "`",            // #
    "...-..-",      // $
    "`",            // %
    ".-...",        // &
    ".----.",       // '
    "`",            // (
    "`",            // )
    "`",            // *
    ".-.-.",        // +
    "--..--",       // ,
    "-....-",       // -
    ".-.-.-",       // .
    "-..-.",        // /
    "-----",        // 0
    ".----",        // 1
    "..---",        // 2
    "...--",        // 3
    "....-",        // 4
    ".....",        // 5
    "-....",        // 6
    "--...",        // 7
    "---..",        // 8
    "----.",        // 9
    "---...",       // :
    "`",            // ;
    "`",            // <
    "-...-",        // = (Paragraph)
    "`",            // >
    "..--..",       // ?
    ".--.-.",       // @
    ".-",           // A
    "-...",         // B
    "-.-.",         // C
    "-..",          // D
    ".",            // E
    "..-.",         // F
    "--.",          // G
    "....",         // H
    "..",           // I
    ".---",         // J
    "-.-",          // K
    ".-..",         // L
    "--",           // M
    "-.",           // N
    "---",          // O
    ".--.",         // P
    "--.-",         // Q
    ".-.",          // R
    "...",          // S
    "-",            // T
    "..-",          // U
    "...-",         // V
    ".--",          // W
    "-..-",         // X
    "-.--",         // Y
    "--..",         // Z
};
int mti_len = (sizeof(international_morse) / sizeof(char*));