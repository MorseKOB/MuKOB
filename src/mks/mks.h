/**
 * Morse KOB System/Server structures, enums, typedefs, facilities.
 *
 * Facilities used by the MKOB subsystems and structures, definitions, etc.
 * to comply with the Morse KOB Server and other standard clients.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _MKS_H_
#define _MKS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Code sequences can have a maximum of 50 elements.
 */
#define MKS_CODESEQ_MAX_LEN 50

 /*
  * *** Message format To/From MorseKOB Server ***
  *
  * From PyKOB (https://github.com/MorseKOB/PyKOB)
  *
  * shortPacketFormat = struct.Struct("<hh")  # cmd, wire
  * idPacketFormat = struct.Struct("<hh 128s 4x i i 8x 208x 128s 8x")  # cmd, byts, id, seq, idflag, ver
  * codePacketFormat = struct.Struct("<hh 128s 4x i 12x 51i i 128s 8x")  # cmd, byts, id, seq, code list, n, txt
  *
  * Python Struct format:
  * Format:
  * '<' indicates little-endian byte order
  * ----+-----------+---------------+-----------+
  * C   | Type      | Python type   | C Size    |
  * ----+-----------+---------------+-----------+
  * x   | pad byte  | no value      |           |
  * ----+-----------+---------------+-----------+
  * h   | short     | integer       | 2         |
  * ----+-----------+---------------+-----------+
  * i   | int       | integer       | 4         |
  * ----+-----------+---------------+-----------+
  * s   | char[]    | bytes (string)|           |
  * ----+-----------+---------------+-----------+
  * 'x' inserts one NUL byte
  */

#define MKS_CMD_DISCONNECT    2  // Disconnect
#define MKS_CMD_DATA          3  // Code or ID
#define MKS_CMD_CONNECT       4  // Connect
#define MKS_CMD_ACK           5  // Ack

#define MKS_CODE_PKT_SIZE 492 // This is from the beginning of the ID to the end, not the actual size
#define MKS_ID_PKT_SIZE 492 // This is from the beginning of the ID to the end, not the actual size
#define MKS_ID_FLAG 1  // Flag indicating that message data is an ID
#define MKS_PKT_MAX_CODE_LEN (MKS_CODESEQ_MAX_LEN + 1) // Maximum size of code sequence in packet
#define MKS_PKT_MAX_STRING_LEN 127  // Leave room for a '\0' terminator in a 128 byte field

#define MKS_OP_NOTIMEOUT 0
#define MKS_OP_TIMEOUT (3 * 1000)       // 3 second request/response timeout
#define MKS_KEEP_ALIVE_TIME (5 * 1000) // Time period to send ID to keep us connected

typedef enum _MCODE_SOURCE_ {
    MCODE_SRC_UNKNOWN,
    MCODE_SRC_UI,
    MCODE_SRC_KEY,
    MCODE_SRC_WIRE,
} mcode_source_t;

/**
 * @brief Code element type (4 byte signed integer).
 */
typedef int32_t code_element_t;

extern code_element_t mcode_long_break;

/**
 * @brief Structure to contain a sequence of code elements, the length, and the source.
 * @ingroup mks
 *
 * Code elements are millisecond time values for key down (positive) / key up (negative).
 * (This doesn't exist in morse.py, as Python can give you the length of an array of integers.)
 */
typedef struct _MCODE_SEQ {
    mcode_source_t source;
    int len;
    code_element_t* code_seq;
} mcode_seq_t;

/**
 * @brief Allocate a mcode_seq_t structure and the code sequence in it. Copy the code sequence into it and set the len.
 * @ingroup mks
 *
 * This retrieves a structure instance from a pool and fills it with the values.
 *
 * @see `mcode_seq_free`
 *
 * @param codeseq The code sequence (int32_t*) to copy (may be NULL).
 * @param len  The length of the code sequence.
 * @return mcode_seq_t* Pointer to an allocated and initialized mcode_seq_t.
 */
extern mcode_seq_t* mcode_seq_alloc(mcode_source_t source, code_element_t* codeseq, int len);

/**
 * @brief Append a code sequence to an existing mcode_seq_t instance.
 * @ingroup mks
 *
 * This will append up to the maximum length allowed in an mcode_seq_t.
 *
 * @param mcode_seq The mcode structure instance to append to.
 * @param codeseq The code sequence to append.
 * @param len The length of the code sequence.
 * @return The number of code elements appended.
 */
extern int mcode_seq_append(mcode_seq_t* mcode_seq, code_element_t* codeseq, int len);

/**
 * @brief Allocate a mcode_seq_t structure and copy an existing mcode_seq_t instance into it.
 * @ingroup mks
 *
 * @param mcode_seq The mcode structure to copy.
 * @return mcode_seq_t* The copy.
 */
extern mcode_seq_t* mcode_seq_copy(const mcode_seq_t* mcode_seq);

/**
 * @brief Free an allocated mcode_seq_t.
 * @ingroup mks
 *
 * This frees the code sequence (returns it to the pool).
 *
 * @param mcode Pointer to the mcode_seq_t to free.
 */
extern void mcode_seq_free(mcode_seq_t* mcode_seq);

/**
 * @brief Initialize the MKS module for use.
 * @ingroup mks
 */
extern void mks_module_init();

#ifdef __cplusplus
}
#endif
#endif // _MKS_H_
