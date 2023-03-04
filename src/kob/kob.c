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
