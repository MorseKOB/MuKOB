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
#ifndef _MORSE_H_
#define _MORSE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "config.h"

#define DOTS_PER_WORD 45 // Dot units per word, including all spaces (MORSE is 43, PARIS is 47)
#define UNIT_DOT_TIME 1200 // Dot time/duration (milliseconds) at 1 Word Per Minute

#define SP ' '

/**
 * @brief Structure to contain a list of code elements and the length of the list.
 * @ingroup morse
 *
 * Code elements are millisecond time values for key down / key up.
 * (This doesn't exist in morse.py, as Python can give you the length of an array of integers.)
 */
typedef struct _MCODE_ {
    int len;
    int32_t* code;
} mcode_t;

/**
 * @brief Allocate a mcode_t structure and the code list in it. Copy the code list into it and set the len.
 * @ingroup morse
 *
 * @see `mcode_free`
 *
 * @param codelist The code list (int32_t*) to allocate for and copy.
 * @param len  The length of the code list.
 * @return mcode_t* Pointer to an allocated and initialized mcode_t.
 */
extern mcode_t* mcode_alloc(int32_t* codelist, int len);

/**
 * @brief Free an allocated mcode_t.
 * @ingroup morse
 *
 * This frees the code list and then frees the mcode_t itself.
 *
 * @param mcode Pointer to the mcode_t to free.
 */
extern void mcode_free(mcode_t* mcode);

/**
 * @brief Encode a character into a list of code elements.
 * @ingroup morse
 *
 * This encodes a character into a list of code elements based on the configuration
 * that was set in the `morse_init`.
 *
 * @param c The character to encode.
 * @return mcode_t Structure of code elements. The structure and the code list must be free'd.
 */
extern mcode_t* morse_encode(char c);

/**
 * @brief Initialize the encoder/decoder with Morse parameters to use.
 * @ingroup morse
 *
 * This sets values for encoding and decoding Morse. It can be called at any point to change these
 * parameters, but when called will terminate any encoding/decoding that is in process.
 *
 * @param twpm Text speed in words per minute.
 * @param cwpm_min Character speed minimum in words per minute. This is used for Farnsworth timing.
 * @param code_type Code type to use - American or International
 * @param spacing Where to insert space for Farnsworth timing (None, between characters, between words).
 */
extern void morse_init(uint8_t twpm, uint8_t cwpm_min, code_type_t code_type, code_spacing_t spacing);

#ifdef __cplusplus
    }
#endif
#endif // _MORSE_H_
