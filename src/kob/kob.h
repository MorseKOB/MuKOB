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
#include "config.h"

/**
 * @brief Initialize the KOB functionality.
 * @ingroup kob
 *
 * @param config configuration to use to initialize the kob module.
 */
void kob_init(const config_t* config);

/**
 * @brief Read the key and return `true` if it is closed.
 * @ingroup kob
 *
 * @return true if closed
 * @return false if open
 */
bool kob_key_is_closed(void);

/**
 * @brief Energize/deenergize the sounder
 * @ingroup kob
 *
 * Energizing the sounder makes the 'click'. Deenergizing makes the 'clack'.
 *
 * @param energize True to energize, False to deenergize.
 */
void kob_sounder_energize(bool energize);

#ifdef __cplusplus
    }
#endif
#endif // _KOB_H_
