/**
 * KOB Configuration File Operations
 *
 * This is intended for use by the `config` functionality, rather than as a
 * general interface. It provides the disk (SD) operations.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */

#include "config_fops.h"

#include "mkboard.h"
#include "util.h"

#include "ff.h"
#include "sd_card.h"
#include "string.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <stdio.h>

static bool _initialized;

static const char* _sys_cfg_filename = "mukob.sys.cfg";
#define _CFG_FILENAME_FORMAT "mukob.%hu.cfg"

static const struct _SYS_CFG_ITEM_HANDLER_CLASS_** _sys_cfg_handlers;
static const cfg_item_handler_class_t** _cfg_handlers;
static FATFS _fs;

static FRESULT _cfo_mount_sd() {
    FRESULT res = FR_OK;

    if (_fs.fs_type == 0) {
        res = f_mount(&_fs, "0:", 1);
        if (FR_OK != res) {
            error_printf(false, "Config - Could not mount SD: (Error: %d)\r\n", res);
        }
    }

    return (res);
}

static FRESULT _cfo_unmount_sd() {
    FRESULT res = FR_OK;

    if (_fs.fs_type != 0) {
        res = f_unmount("0:");
        _fs.fs_type = 0;
    }

    return (res);
}

int _process_cfg_line(config_t* config, char* line) {
    int retval = 0;
    char* cfgline = strnltonull((char*)strskipws(line));
    char* key;
    char* value;
    const char* eq = "=";

    if (*cfgline == '\000' || *cfgline == '#') {
        // It's blank or a comment line. Nothing to do.
        return (retval);
    }

    // Use strtok_r rather than strtok in case the other core needs to tokenize something.
    key = strtok_r(line, eq, &line);
    if (NULL != key) {
        value = strtok_r(line, eq, &line);
    }

    // Run through the handlers and see if one handles it...
    const cfg_item_handler_class_t** handlers = _cfg_handlers;
    while (*handlers) {
        const cfg_item_handler_class_t* handler = *handlers;
        if (strcmp(handler->key, key) == 0) {
            retval = handler->reader(handler, config, value);
            return (retval);
        }
        handlers++;
    }
    // No handler found for this key
    retval = (-1);

    warn_printf("Config - Unknown key: '%s'\n", key);
    return (retval);
}

FRESULT cfo_read_cfg(config_t* cfg, uint16_t cfg_num) {
    FRESULT fr = FR_OK;
    FIL fil;
    char buf[100];
    char cfg_file_name[16];

    // Build the filename using the config number.
    sprintf(cfg_file_name, _CFG_FILENAME_FORMAT, cfg_num);

    // Mount drive
    fr = _cfo_mount_sd();
    if (fr != FR_OK) {
        return (fr);
    }

    fr = f_open(&fil, cfg_file_name, FA_READ);
    if (FR_OK != fr) {
        error_printf(false, "Config - Could not open file '%s' (Error: %d).\n", cfg_file_name, fr);
        return (fr);
    }
    while (f_gets(buf, sizeof(buf), &fil)) {
        if (_process_cfg_line(cfg, buf) < 0) {
            return (FR_INVALID_PARAMETER);
        }
    }
    // Close file and unmount the SD
    fr = f_close(&fil);
    _cfo_unmount_sd();

    return (fr);
}

uint16_t _process_sys_cfg_line(config_sys_t* sys_cfg, char* line) {
    uint16_t retval = 0; // Bit ID of the item set
    char* cfgline = strnltonull((char*)strskipws(line));
    char* key;
    char* value;
    const char* eq = "=";

    if (*cfgline == '\000' || *cfgline == '#') {
        // It's blank or a comment line. Nothing to do.
        return (retval);
    }

    // Use strtok_r rather than strtok incase the other core needs to tokenize something.
    key = strtok_r(line, eq, &line);
    if (NULL != key) {
        value = strtok_r(line, eq, &line);
    }


    // Run through the handlers and see if one handles it...
    const sys_cfg_item_handler_class_t** handlers = _sys_cfg_handlers;
    while (*handlers) {
        const sys_cfg_item_handler_class_t* handler = *handlers;
        if (strcmp(handler->key, key) == 0) {
            if (handler->reader(handler, sys_cfg, value) > 0) {
                return (handler->id_flag); // Indicate that we set this item
            }
            else {
                return (0); // Indicate that we didn't set this item
            }
        }
        handlers++;
    }
    // No handler found for this key
    warn_printf("System Config - Unknown key: '%s'\n", key);
    return (retval);
}

uint16_t cfo_read_sys_cfg(config_sys_t * sys_cfg) {
    uint16_t not_init = (
        _SYSCFG_NOT_LOADED
        | _SYSCFG_VER_ID
        | _SYSCFG_BCN_ID
        | _SYSCFG_TZ_ID
        | _SYSCFG_WS_ID
        ); // Will clear as set
    FRESULT fr;
    FIL fil;
    char buf[100];

    // Mount drive
    fr = _cfo_mount_sd();
    if (fr != FR_OK) {
        return (not_init);
    }

    // Read the system config file
    fr = f_open(&fil, _sys_cfg_filename, FA_READ);
    if (fr != FR_OK) {
        error_printf(false, "Config - Could not open file (%d)\r\n", fr);
        return (not_init);
    }
    while (f_gets(buf, sizeof(buf), &fil)) {
        not_init ^= _process_sys_cfg_line(sys_cfg, buf);
    }
    not_init &= !_SYSCFG_NOT_LOADED;
    // Close file
    fr = f_close(&fil);
    // See if we got values for all needed settigs
    bool is_set = !(binary_from_int(not_init));
    if ((not_init & _SYSCFG_VER_ID) || CONFIG_VERSION != sys_cfg->cfg_version) {
        not_init |= _SYSCFG_VER_ID;
        is_set = false;
    }
    if ((not_init & _SYSCFG_BCN_ID) || (sys_cfg->boot_cfg_number < 1 || sys_cfg->boot_cfg_number > 9)) {
        not_init |= _SYSCFG_BCN_ID;
        is_set = false;
    }
    if ((not_init & _SYSCFG_TZ_ID) || (sys_cfg->tz_offset < -12.0 || sys_cfg->tz_offset > 14.0)) {
        not_init |= _SYSCFG_TZ_ID;
        sys_cfg->tz_offset = 0.0;
        is_set = false;
    }
    if ((not_init & _SYSCFG_WS_ID) || !sys_cfg->wifi_ssid) {
        not_init |= _SYSCFG_WS_ID;
        is_set = false;
    }
    if ((not_init & _SYSCFG_WP_ID) || !sys_cfg->wifi_password) {
        not_init |= _SYSCFG_WP_ID;
        is_set = false;
    }
    sys_cfg->is_set = is_set;

    _cfo_unmount_sd();

    return (not_init);
}

extern FRESULT cfo_save_cfg(const config_t* cfg, uint16_t cfg_num) {
    FRESULT fr = FR_OK;
    FIL fil;
    char cfg_file_name[16];
    char buf[256];

    // Mount drive
    fr = _cfo_mount_sd();
    if (fr != FR_OK) {
        return (fr);
    }

    // Create/Open-Truncate file for writing.
    snprintf(cfg_file_name, sizeof(cfg_file_name), _CFG_FILENAME_FORMAT, cfg_num);
    fr = f_open(&fil, cfg_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fr) {
        error_printf(false, "Config - Could not open file '%s' (Error: %d).\n", cfg_file_name, fr);
        _cfo_unmount_sd();
        return (fr);
    }
    // Write the config values out
    const cfg_item_handler_class_t** handlers = _cfg_handlers;
    while (*handlers) {
        unsigned int bytes_written = 0;
        const cfg_item_handler_class_t* handler = *handlers;
        int len = handler->writer(handler, cfg, buf, true);
        len += sprintf(buf + len, "\n");
        fr = f_write(&fil, buf, len, &bytes_written);
        if (FR_OK != fr || len != bytes_written) {
            if (FR_OK == fr) {
                error_printf(false, "Config - Writting config %s. Bytes expected: %d  Written: %d", cfg_file_name, len, bytes_written);
                fr = FR_INVALID_PARAMETER;
            }
            break;
        }
        handlers++;
    }

    f_close(&fil);
    _cfo_unmount_sd();

    return (fr);
}

FRESULT cfo_save_sys_cfg(const config_sys_t* sys_cfg) {
    FRESULT fr = FR_OK;
    FIL fil;
    char buf[256];

    // Mount drive
    fr = _cfo_mount_sd();
    if (fr != FR_OK) {
        return (fr);
    }

    // Create/Open-Truncate file for writing.
    fr = f_open(&fil, _sys_cfg_filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fr) {
        error_printf(false, "Config - Could not open file '%s' (%d).\n", _sys_cfg_filename, fr);
        _cfo_unmount_sd();
        return (fr);
    }
    // Write the config values out
    const sys_cfg_item_handler_class_t** handlers = _sys_cfg_handlers;
    while (*handlers) {
        unsigned int bytes_written = 0;
        const sys_cfg_item_handler_class_t* handler = *handlers;
        int len = handler->writer(handler, sys_cfg, buf, true);
        len += sprintf(buf + len, "\n");
        fr = f_write(&fil, buf, len, &bytes_written);
        if (FR_OK != fr || len != bytes_written) {
            if (FR_OK == fr) {
                error_printf(false, "Config - Writting config %s. Bytes expected: %d  Written: %d", _sys_cfg_filename, len, bytes_written);
                fr = FR_INVALID_PARAMETER;
            }
            break;
        }
        handlers++;
    }

    f_close(&fil);
    _cfo_unmount_sd();

    return (fr);
}

void config_fops_module_init(const sys_cfg_item_handler_class_t** sys_cfg_handlers, const cfg_item_handler_class_t** cfg_handlers) {
    assert(!_initialized);
    _sys_cfg_handlers = sys_cfg_handlers;
    _cfg_handlers = cfg_handlers;
    sd_init_driver();
    _fs.fs_type = 0;

    _initialized = true;
}