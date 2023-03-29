/**
 * MuKOB Morse conversion tables - American and International
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _MORSE_TABLES_H_
#define _MORSE_TABLES_H_
#ifdef __cplusplus
  extern "C" {
#endif

/**
 * @brief Table of dit-dah strings for characters from Space through Z.
 * @ingroup morse-table
 *
 * This table includes some characters that have no Morse characters in
 * order to make it easier to do lookups. These are in ASCII code
 * order, so indexing in from a character, or determining a character
 * from the code is trivial.
 *
 * The value is a string with:
 * ' ' - Intra-character space
 * '.' - Dit/Dot
 * '-' - Dah/Dash
 * '=' - Long-Dah
 * '+' - Special marker: Close
 * '`' - Special marker: Undefined char
 *
 * For example:
 *  A = ".-"
 *  B = "-..."
 *  C = ".. ."
 */
extern const char* american_morse[];

/**
 * @brief Table of dit-dah strings for characters from Space through Z.
 * @ingroup morse-table
 *
 * This table includes some characters that have no Morse characters in
 * order to make it easier to do lookups. These are in ASCII code
 * order, so indexing in from a character, or determining a character
 * from the code is trivial.
 *
 * The value is a string with:
 * '.' - Dit/Dot
 * '-' - Dah/Dash
 *
 * For example:
 *  A = ".-"
 *  B = "-..."
 *  C = ".-.-"
 */
extern const char* international_morse[];

#ifdef __cplusplus
  }
#endif
#endif // _MORSE_TABLES_H_
