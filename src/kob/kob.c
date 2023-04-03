/**
 * MuKOB Key On Board (key & sounder) functionality.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "kob.h"
#include "config.h"
#include "system_defs.h"

#define _KOB_DEBOUNCE 0.015     // time to ignore transitions due to contact bounce(sec)
#define _KOB_CODE_SPACE 0.120   // amount of space to signal end of code sequence(sec)
#define _KOB_CKT_CLOSE 0.800    // length of mark to signal circuit closure(sec)

static bool _key_closer_is_open = false;
static bool _key_last_state = false; // 'false' means open
static bool _sounder_energized = false;

void kob_init(const config_t* config) {
    kob_sounder_energize(true);
}

void kob_sounder_energize(bool energize) {
    gpio_put(KOB_SOUNDER_OUT, (energize ? KOB_SOUNDER_ENERGIZED : KOB_SOUNDER_DEENERGIZED));
    _sounder_energized = energize;
}
