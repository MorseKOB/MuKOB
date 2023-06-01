/**
 * MuKOB Key On Board (KOB) data types.
 *
 * This contains structure, enum, typedefs, to avoid circular inclusions
 * of header files.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _KOB_T_H_
#define _KOB_T_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief State of the read code from key processing.
 */
enum _KEY_READ_PHASE_ {
    KEY_READ_START,
    KEY_READ_DEBOUNCE,
    KEY_READ_CONTINUE,
    KEY_READ_COMPLETE,
};

typedef struct _KEY_READ_STATE_ {
    enum _KEY_READ_PHASE_ phase;
    uint32_t delta_time;
} key_read_state_t;

/**
 * @brief Status of the KOB and loop.
 * @ingroup kob
 *
 * Status values pertaining to the Key, Sounder, Tone, and Loop, reported to
 * other modules.
 */
typedef struct _KOB_STATUS_ {
    volatile bool circuit_closed;
    volatile bool key_closed;
    volatile bool sounder_energized;
    volatile bool tone_energized;
} kob_status_t;

#ifdef __cplusplus
}
#endif
#endif // _KOB_T_H_
