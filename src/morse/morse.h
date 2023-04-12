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
#include "float.h"

#include "config.h"

// Used for Encoding
#define DOTS_PER_WORD 45        // Dot units per word, including all spaces (MORSE is 43, PARIS is 47)
#define UNIT_DOT_TIME 1200      // Dot time (duration in milliseconds) at 1 Word Per Minute
#define SP ' '
// Used for Decoding
#define MD_ALPHA 0.5               // weight given to wpm update values(for smoothing)
#define MD_MIN_DASH_LEN 1.5        // dot vs dash threshold(in dots)
#define MD_MAX_DASH_LEN 9.0        // long dash vs circuit closure threshold(in dots)
#define MD_MIN_MORSE_SPACE 2.0     // intra-symbol space vs Morse(in dots)
#define MD_MAX_MORSE_SPACE 6.0     // maximum length of Morse space(in dots)
#define MD_MIN_CHAR_SPACE 2.7      // intra-symbol space vs character space(in dots)
#define MD_MIN_L_LEN 5.0           // minimum length of L character(in dots)
#define MD_MORSE_RATIO 0.95        // length of Morse space relative to surrounding spaces

#define MORSE_EXTENDED_MARK_START_INDICATOR 1   // Closer/Circuit closed indicator
#define MORSE_EXTENDED_MARK_END_INDICATOR 2     // Closer/Circuit open indicator

#define MORSE_MAX_DDS_IN_CHAR 9 // Maximum number of dits, dahs (& intra-char-space) in a character

#define MORSE_CODE_ELEMENT_VALUE_MAX (FLT_MAX)

typedef enum _MCODE_SEQ_SOURCE_ {
    MCODE_SRC_UI,
    MCODE_SRC_KEY,
    MCODE_SRC_WIRE,
} mcode_seq_source_t;

/**
 * @brief Structure to contain a sequence of code elements and the length of the sequence.
 * @ingroup morse
 *
 * Code elements are millisecond time values for key down (positive) / key up (negative).
 * (This doesn't exist in morse.py, as Python can give you the length of an array of integers.)
 */
typedef struct _MCODE_SEQ {
    mcode_seq_source_t source;
    int len;
    int32_t* code_seq;
} mcode_seq_t;

/**
 * @brief Allocate a mcode_seq_t structure and the code sequence in it. Copy the code sequence into it and set the len.
 * @ingroup morse
 *
 * @see `mcode_seq_free`
 *
 * @param codeseq The code sequence (int32_t*) to allocate for and copy.
 * @param len  The length of the code sequence.
 * @return mcode_seq_t* Pointer to an allocated and initialized mcode_seq_t.
 */
extern mcode_seq_t* mcode_seq_alloc(mcode_seq_source_t source, int32_t* codeseq, int len);

/**
 * @brief Allocate a mcode_seq_t structure and copy an existing mcode_seq_t instance into it.
 * @ingroup morse
 *
 * @param mcode_seq The mcode structure to copy.
 * @return mcode_seq_t* The copy.
 */
extern mcode_seq_t* mcode_seq_copy(mcode_seq_t* mcode_seq);

/**
 * @brief Free an allocated mcode_seq_t.
 * @ingroup morse
 *
 * This frees the code sequence and then frees the mcode_seq_t itself.
 *
 * @param mcode Pointer to the mcode_seq_t to free.
 */
extern void mcode_seq_free(mcode_seq_t* mcode_seq);

/**
 * @brief Decode a Morse Code sequence to text.
 * @ingroup morse
 *
 * @param mcode_seq Morse Code sequence to decode.
 */
void morse_decode(mcode_seq_t* mcode_seq);

/**
 * @brief Message handler for 'MSG_MORSE_DECODE_FLUSH' (alarm timeout) to flush a pending decode operation.
 * @ingroup morse
 */
extern void morse_decode_flush();

/**
 * @brief Encode a character into a list of code elements.
 * @ingroup morse
 *
 * This encodes a character into a list of code elements based on the configuration
 * that was set in the `morse_module_init`.
 *
 * @param c The character to encode.
 * @return mcode_seq_t Structure of code elements. The structure and the code list must be free'd.
 */
extern mcode_seq_t* morse_encode(char c);

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
extern void morse_module_init(uint8_t twpm, uint8_t cwpm_min, code_type_t code_type, code_spacing_t spacing);

#ifdef __cplusplus
    }
#endif
#endif // _MORSE_H_
