/**
 * MuKOB Key On Board (key & sounder) functionality.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#ifndef _KOB_H_
#define _KOB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "kob_t.h"
#include "config.h"
#include "cmt.h"

/**
 * @brief Read the key and return `true` if it is closed.
 * @ingroup kob
 *
 * @return true if closed
 * @return false if open
 */
extern bool kob_key_is_closed(void);

/**
 * @brief Message handler to start/continue/finish reading code from the key.
 * @ingroup kob
 *
 * This is a message handler, so it does not block. As code is collected, a
 * message is posted with the code sequence.
 */
extern void kob_read_code_from_key(cmt_msg_t* msg);

/**
 * @brief Sound the code sequence on the sounder and/or speaker.
 *
 * @param mcode_seq The mcode_seq_t morse code sequence to sound.
 */
extern void kob_sound_code(mcode_seq_t* mcode_seq);

/**
 * @brief Continuation of the `kob_sound_code` method - called in response to
 *        MSG_KOB_SOUND_CODE_CONT message.
 */
extern void kob_sound_code_continue();

/**
 * @brief Energize/deenergize the sounder
 * @ingroup kob
 *
 * Energizing the sounder makes the 'click'. Deenergizing makes the 'clack'.
 *
 * @param energize True to energize, False to deenergize.
 */
extern void kob_sounder_energize(bool energize);

/**
 * @brief Initialize the KOB functionality.
 * @ingroup kob
 *
 * Sets the configuration and starts the loop to read code from the key.
 *
 * @param invert_key_input True if the key input should be inverted (used for modem input)
 * @param key_has_closer True if the key has a circuit closer that should be followed
 */
extern void kob_module_init(bool invert_key_input, bool key_has_closer);

#ifdef __cplusplus
    }
#endif
#endif // _KOB_H_
