/**
 * KOB Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 * MuKOB Configuration functionality
 */

#include <stdio.h>
#include "config.h"
#include "pico/stdlib.h"
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

const char* _cfg_filename = ".mukob.cfg";

config_t* _current_cfg;

void _process_line(config_t* config, char* line) {
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
    else if (strcmp(key, "wifi_ssid") == 0) {
        config->wifi_ssid = config_value_create(value);
    }
    else if (strcmp(key, "wifi_pw") == 0) {
        config->wifi_password = config_value_create(value);
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
        free(cfg->wifi_password);
        free(cfg->wifi_ssid);
        // Now free up the main config structure
        free(cfgwm);
    }
}

int config_init(void) {
    config_t* config = config_new(NULL);
    _current_cfg = config;

    // See if we can read the config from the '.cfg' file...
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret = 0;
    char buf[100];

    // Initialize SD card
    do {  // loop structure to provide breaking out at any step during init...
        if (sd_init_driver()) {
            // Mount drive
            fr = f_mount(&fs, "0:", 1);
            if (fr != FR_OK) {
                ret = 1;
                error_printf("Config: Could not mount filesystem (%d)\r\n", fr);
                break;
            }

            // Open file for reading
            fr = f_open(&fil, _cfg_filename, FA_READ);
            if (fr != FR_OK) {
                ret = 2;
                error_printf("Config: Could not open file (%d)\r\n", fr);
                break;
            }

            // Print every line in file over serial
            debug_printf("Reading from file '%s':\r\n", _cfg_filename);
            debug_printf("---\r\n");
            while (f_gets(buf, sizeof(buf), &fil)) {
                debug_printf(buf);
                _process_line(config, buf);
            }
            debug_printf("\r\n---\r\n");

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
    } while(true); // Loop structure to allow break out to following code at any point

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
            if (NULL != init_values->wifi_password) {
                cfg->wifi_password = config_value_create(init_values->wifi_password);
            }
            if (NULL != init_values->wifi_ssid) {
                cfg->wifi_ssid = config_value_create(init_values->wifi_ssid);
            }
        }
    }

    return (cfg);
}

char* config_value_create(char* value) {
    char* malloced_value;
    malloced_value = malloc(strlen(value) + 1);
    strcpy(malloced_value, value);
    return (malloced_value);
}
