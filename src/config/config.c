/**
 * KOB Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * MuKOB Configuration functionality
 */
#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "sd_card.h"
#include "string.h"
#include "ff.h"
#include "mkboard.h"
#include "net.h"
#include "util.h"

#define _CFG_MEM_MARKER_ 3224
typedef struct _CFG_W_MARKER {
    uint16_t marker;
    config_t config;
} _cfg_w_marker_t;

const char* _sys_cfg_filename = "mukob.sys.cfg";
char* _cfg_filename = NULL;

static config_sys_t _system_cfg = { 1, false, 0.0, NULL, NULL, NULL };
static config_t* _current_cfg;

void _process_cfg_line(config_t* config, char* line) {
    char* cfgline = strnltonull(strskipws(line));
    char *key;
    char *value;
    int iv;
    const char* eq = "=";

    if (*cfgline == '\000' || *cfgline == '#') {
        // It's blank or a comment line. Nothing to do.
        return;
    }

    // Use strtok_r rather than strtok incase the other core needs to tokenize something.
    key = strtok_r(line, eq, &line);
    if (NULL != key) {
        value = strtok_r(line, eq, &line);
    }

    // Not very elegent, but not done much...
    if (strcmp(key, "cfg_version") == 0) {
        iv = atoi(value);
        config->cfg_version = (uint16_t)iv;
    }
    else {
        warn_printf("Config - Unknown key: '%s'\n", key);
    }
}

void _process_sys_cfg_line(char* line) {
    char* cfgline = strnltonull(strskipws(line));
    char* key;
    char* value;
    int iv;
    const char* eq = "=";

    if (*cfgline == '\000' || *cfgline == '#') {
        // It's blank or a comment line. Nothing to do.
        return;
    }

    // Use strtok_r rather than strtok incase the other core needs to tokenize something.
    key = strtok_r(line, eq, &line);
    if (NULL != key) {
        value = strtok_r(line, eq, &line);
    }

    // Not very elegent, but not done much...
    if (strcmp(key, "cfg_version") == 0) {
        iv = atoi(value);
        _system_cfg.cfg_version = (uint16_t)iv;
    }
    else if (strcmp(key, "tz_offset") == 0) {
        double dv = atof(value);
        _system_cfg.tz_offset = dv;
    }
    else if (strcmp(key, "ucfg_filename") == 0) {
        // free a current value
        if (_system_cfg.user_cfg_filename) {
            free(_system_cfg.user_cfg_filename);
        }
        _system_cfg.user_cfg_filename = config_value_create(value);
    }
    else if (strcmp(key, "wifi_ssid") == 0) {
        _system_cfg.wifi_ssid = config_value_create(value);
    }
    else if (strcmp(key, "wifi_pw") == 0) {
        _system_cfg.wifi_password = config_value_create(value);
    }
    else {
        warn_printf("Config - Unknown key: '%s'\n", key);
    }
}

const config_t* config_current() {
    return _current_cfg;
}

void config_free(config_t* cfg) {
    // If this is a config object there is a marker one byte before the beginning.
    // This is to keep it from accidentally being freed directly by `free`, as there
    // are contained structures that also need to be freed.
    _cfg_w_marker_t* cfgwm = (_cfg_w_marker_t*)((uint8_t*)cfg - (sizeof(_cfg_w_marker_t) - sizeof(config_t)));
    if (cfgwm->marker == _CFG_MEM_MARKER_) {
        // Okay, we can free things up...
        // First, free allocated values

        // Now free up the main config structure
        free(cfgwm);
    }
}

int config_init(void) {
    // Set default values in the system config
    _system_cfg.cfg_version = CONFIG_VERSION;
    _system_cfg.user_cfg_filename = NULL;
    _system_cfg.wifi_ssid = NULL;
    _system_cfg.wifi_password = NULL;
    // Create a config object to use as the current
    config_t* config = config_new(NULL);
    _current_cfg = config;
    config->cfg_version = CONFIG_VERSION;

    // See if we can read the system and user config from the '.cfg' files...
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret = 0;
    char buf[100];

    // Initialize SD card
    do {  // loop structure to allow breaking out at any step during init...
        if (sd_init_driver()) {
            // Mount drive
            fr = f_mount(&fs, "0:", 1);
            if (fr != FR_OK) {
                ret = 1;
                error_printf("Config: Could not mount filesystem (%d)\r\n", fr);
                break;
            }

            // Read the system config first
            fr = f_open(&fil, _sys_cfg_filename, FA_READ);
            if (fr != FR_OK) {
                ret = 2;
                error_printf("Config: Could not open file (%d)\r\n", fr);
                break;
            }
            while (f_gets(buf, sizeof(buf), &fil)) {
                _process_sys_cfg_line(buf);
            }
            // Close file
            fr = f_close(&fil);
            if (fr != FR_OK) {
                ret = 3;
                error_printf("Config: Could not close file (%d)\r\n", fr);
                break;
            }
            // See if we got values for all needed settigs
            bool is_set = true; // Optimist
            do { // To allow breaking out
                if (CONFIG_VERSION != _system_cfg.cfg_version) {
                    is_set = false;
                    break;
                }
                if (_system_cfg.tz_offset < -12.0 || _system_cfg.tz_offset > 14.0) {
                    _system_cfg.tz_offset = 0.0;
                    is_set = false;
                    break;
                }
                if (!_system_cfg.wifi_ssid) {
                    is_set = false;
                    break;
                }
                if (!_system_cfg.wifi_password) {
                    is_set = false;
                    break;
                }
                break;
            } while(true);
            _system_cfg.is_set = is_set;

            // Now read a user config
            fr = f_open(&fil, _system_cfg.user_cfg_filename, FA_READ);
            if (fr != FR_OK) {
                ret = 2;
                error_printf("Config: Could not open file (%d)\r\n", fr);
                break;
            }
            while (f_gets(buf, sizeof(buf), &fil)) {
                _process_cfg_line(config, buf);
            }
            // Save the filename we used
            if (_cfg_filename) {
                free(_cfg_filename);
            }
            _cfg_filename = config_value_create(_system_cfg.user_cfg_filename);
            // Close file
            fr = f_close(&fil);
            if (fr != FR_OK) {
                ret = 3;
                error_printf("Config: Could not close file (%d)\r\n", fr);
                break;
            }

            // Unmount drive
            f_unmount("0:");
            break;
        }
    } while (0);

    return (ret);
}

config_t* config_new(config_t* init_values) {
    // Allocate memory for a config structure, 'mark' it, and
    // set initial values.
    size_t size = sizeof(_cfg_w_marker_t);
    _cfg_w_marker_t* cfgwm = calloc(1, size);
    config_t* cfg = NULL;
    if (NULL != cfgwm) {
        cfgwm->marker = _CFG_MEM_MARKER_;
        cfg = &(cfgwm->config);
        if (NULL != init_values) {
            cfg->cfg_version = init_values->cfg_version;
        }
    }

    return (cfg);
}

const config_sys_t* config_sys() {
    return (&_system_cfg);
}

bool config_sys_set() {
    return (_system_cfg.is_set);
}

char* config_value_create(char* value) {
    char* malloced_value;
    malloced_value = malloc(strlen(value) + 1);
    strcpy(malloced_value, value);

    return (malloced_value);
}
