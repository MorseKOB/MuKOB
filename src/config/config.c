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
#include "mkwire.h"

#define _CFG_MEM_MARKER_ 3224 // *Magic* & *Air*

/**
 * @brief Holds the 'magic' marker and a config structure to safegaurd free'ing.
 *
 * A config object holds values that are also malloc'ed, and therefore
 * need to be free'ed when the config object is free'ed. To safeguard
 * against a client simply free'ing the config object this structure is malloc'ed
 * and initialized, and then the contained config object is made available to clients.
 * This prevents clients from accedentally free'ing the config object
 * (using `free(config*)`), as it will cause a fault. Rather, the `config_free()`
 * method must be used. It correctly free's the additional malloc'ed objects as well
 * as the main object.
 */
typedef struct _CFG_W_MARKER {
    uint16_t marker;
    config_t config;
} _cfg_w_marker_t;

/**
 * @brief Config item handler type. Functions of this type used to process config file lines.
 * @ingroup config
 *
 * Defines the signature of config item handlers. The handler is overloaded to be able
 * to handle processing lines read from a config file into a config object and process a config
 * object to write lines out to a config file.
 *
 * Operation:
 *  Reading a config file:
 *    The functions are in a list and are tried one by one on a line read from a
 *    config file. If the handler knows how to handle the passed in key, it processes the value
 *    into the config object and returns >0. If the handler is not for the passed in key it
 *    must return 0. If it is for the key, but encounters an error it should return <0.
 *    NOTE: If the value string is to be stored into the config object `config_value_create` should
 *          be used to allocate and copy the value.
 *
 *   Writing a config file:
 *    The function is called with a config and buffer to fill. The handler processes the piece of data
 *    from the config object and writes it into the buffer. It returns the number of characters written
 *    into the buffer.
 *
 *  @param cfg The config object to put values into (Read operation) or get values from (Write operation)
 *  @param key The string 'key' from a line of a config file (Read operation). NULL (Write operation)
 *  @param value The string 'value' from a line of a config file (Read operation). NULL (Write operation)
 *  @param buf The buffer to write the key=value into. NULL (Read operation)
 */
typedef int32_t (*cfg_item_handler_fn)(config_t* cfg, const char* key, const char* value, char* buf);

static int32_t _cih_config_version(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_auto_connect(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_code_type(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_key_has_closer(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_local(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_char_speed_min(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_remote(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_server_url(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_sound(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_sounder(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_spacing(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_station(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_text_speed(config_t* cfg, const char* key, const char* value, char* buf);
static int32_t _cih_wire(config_t* cfg, const char* key, const char* value, char* buf);

/**
 * @brief Array of config item handlers.
 * @ingroup config
 *
 * When processing a config file into a config object, each of these is called until one returns non-zero
 * (which means that it handled it (>0 success, <0 error)).
 *
 * When processing a config object into a config file (writing to the file), each handler is called and is
 * expected to write the value it handles into the file.
 *
 * The handlers should be in the order that the config lines should show up in the config file.
 */
static const cfg_item_handler_fn cfg_handlers[] = {
    _cih_config_version,
    _cih_auto_connect,
    _cih_code_type,
    _cih_key_has_closer,
    _cih_local,
    _cih_char_speed_min,
    _cih_remote,
    _cih_server_url,
    _cih_sound,
    _cih_sounder,
    _cih_spacing,
    _cih_station,
    _cih_text_speed,
    _cih_wire,
    ((cfg_item_handler_fn)0), // NULL last item to signify end
};

/**
 * @brief System config item handler type. Functions of this type used to process system config file lines.
 * @ingroup config
 *
 * Defines the signature of system config item handlers. The handler is overloaded to be able
 * to handle processing lines read from a system config file into a system config object and process a
 * system config object to write lines out to a system config file.
 *
 * Operation:
 *  Reading a system config file:
 *    The functions are in a list and are tried one by one on a line read from a system
 *    config file. If the handler knows how to handle the passed in key, it processes the value
 *    into the system config object and returns >0. If the handler is not for the passed in key it
 *    must return 0. If it is for the key, but encounters an error it should return <0.
 *    NOTE: If the value string is to be stored into the system config object `config_value_create` should
 *          be used to allocate and copy the value.
 *
 *   Writing a system config file:
 *    The function is called with a system config and buffer to fill. The handler processes the piece of data
 *    from the system config object and writes it into the buffer. It returns the number of characters written
 *    into the buffer.
 *
 *  @param sys_cfg The system config object to put values into (Read operation) or get values from (Write operation)
 *  @param key The string 'key' from a line of a system config file (Read operation). NULL (Write operation)
 *  @param value The string 'value' from a line of a system config file (Read operation). NULL (Write operation)
 *  @param buf The buffer to write the key=value into. NULL (Read operation)
 */
typedef int32_t(*sys_cfg_item_handler_fn)(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);

static int32_t _scih_config_version(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);
static int32_t _scih_tz_offset(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);
static int32_t _scih_user_cfg_filename(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);
static int32_t _scih_wifi_password(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);
static int32_t _scih_ssid(config_sys_t* sys_cfg, const char* key, const char* value, char* buf);

static const sys_cfg_item_handler_fn sys_cfg_handlers[] = {
    _scih_config_version,
    _scih_tz_offset,
    _scih_user_cfg_filename,
    _scih_wifi_password,
    _scih_ssid,
    ((sys_cfg_item_handler_fn)0), // NULL last item to signify end
};

const char* _sys_cfg_filename = "mukob.sys.cfg";
char* _current_cfg_filename = NULL;

static config_sys_t _system_cfg = { 1, false, 0.0, NULL, NULL, NULL };
static config_t* _current_cfg;

static int32_t _cih_config_version(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "cfg_version";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            int iv = atoi(value);
            cfg->cfg_version = (uint16_t)iv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 10 + 2; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n\n", our_key, cfg->cfg_version);
    }
    return (retval);
}

static int32_t _cih_auto_connect(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "auto_connect";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->auto_connect = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->auto_connect));
    }
    return (retval);
}

static const char* _code_type_enum_names[] = {
    "AMERICAN",
    "INTERNATIONAL",
};

static int32_t _cih_code_type(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "code_type";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            retval = -1;
            for (int i = 0; i < sizeof(_code_type_enum_names); i++) {
                if (strcmp(_code_type_enum_names[i], value) == 0) {
                    cfg->code_type = (code_type_t)i;
                        retval = 1;
                        break;
                }
            }
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 13 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, _code_type_enum_names[cfg->code_type]);
    }
    return (retval);
}

static int32_t _cih_key_has_closer(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "key_has_closer";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->key_has_closer = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->key_has_closer));
    }
    return (retval);
}

static int32_t _cih_local(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "local";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->local = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->local));
    }
    return (retval);
}

static int32_t _cih_char_speed_min(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "char_speed_min";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            int iv = atoi(value);
            cfg->char_speed_min = (uint8_t)iv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 10 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, cfg->char_speed_min);
    }
    return (retval);
}

static int32_t _cih_remote(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "remote";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->remote = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->remote));
    }
    return (retval);
}

static int32_t _cih_server_url(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "server_url";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            if (cfg->server_url) {
                free(cfg->server_url);
            }
            cfg->server_url = config_value_create(value);
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + NET_URL_MAX_LEN + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, cfg->server_url);
    }
    return (retval);
}

static int32_t _cih_sound(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "sound";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->sound = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->sound));
    }
    return (retval);
}

static int32_t _cih_sounder(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "sounder";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            bool b = bool_from_str(value);
            cfg->sounder = b;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 1 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, binary_from_bool(cfg->sounder));
    }
    return (retval);
}

static const char* _spacing_enum_names[] = {
    "NONE",
    "CHAR",
    "WORD",
};

static int32_t _cih_spacing(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "spacing";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            retval = -1;
            for (int i = 0; i < sizeof(_spacing_enum_names); i++) {
                if (strcmp(_spacing_enum_names[i], value) == 0) {
                    cfg->spacing = (code_spacing_t)i;
                    retval = 1;
                    break;
                }
            }
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 4 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, _spacing_enum_names[cfg->spacing]);
    }
    return (retval);
}

static int32_t _cih_station(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "station";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            if (cfg->station) {
                free(cfg->server_url);
            }
            cfg->server_url = config_value_create(value);
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + MKOBSERVER_STATION_ID_MAX_LEN + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, cfg->server_url);
    }
    return (retval);
}

static int32_t _cih_text_speed(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "text_speed";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            int iv = atoi(value);
            cfg->text_speed = (uint8_t)iv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 10 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, cfg->text_speed);
    }
    return (retval);
}

static int32_t _cih_wire(config_t* cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "wire";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            int iv = atoi(value);
            cfg->wire = (uint8_t)iv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 10 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n", our_key, cfg->wire);
    }
    return (retval);
}

static int32_t _scih_config_version(config_sys_t* sys_cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "cfg_version";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            int iv = atoi(value);
            sys_cfg->cfg_version = (uint16_t)iv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 10 + 2; // key=value\n
        retval = snprintf(buf, max_len, "%s=%hd\n\n", our_key, sys_cfg->cfg_version);
    }
    return (retval);
}

static int32_t _scih_tz_offset(config_sys_t* sys_cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "tz_offset";

    if (key) {
        if (strcmp(key, our_key) == 0) {
            double dv = atof(value);
            sys_cfg->tz_offset = dv;
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + 5 + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%.1f\n", our_key, sys_cfg->tz_offset);
    }
    return (retval);
}

static int32_t _scih_user_cfg_filename(config_sys_t* sys_cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "ucfg_filename";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            if (sys_cfg->user_cfg_filename) {
                free(sys_cfg->user_cfg_filename);
            }
            sys_cfg->user_cfg_filename = config_value_create(value);
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + FF_MAX_LFN + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, sys_cfg->user_cfg_filename);
    }
    return (retval);
}

static int32_t _scih_wifi_password(config_sys_t* sys_cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "wifi_pw";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            if (sys_cfg->wifi_password) {
                free(sys_cfg->wifi_password);
            }
            sys_cfg->wifi_password = config_value_create(value);
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + NET_PASSWORD_MAX_LEN + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, sys_cfg->wifi_password);
    }
    return (retval);
}

static int32_t _scih_ssid(config_sys_t* sys_cfg, const char* key, const char* value, char* buf) {
    int32_t retval = 0;
    char* our_key = "wifi_ssid";

    if (key) {
        // See if it is the key we handle
        if (strcmp(key, our_key) == 0) {
            if (sys_cfg->wifi_ssid) {
                free(sys_cfg->wifi_ssid);
            }
            sys_cfg->wifi_ssid = config_value_create(value);
            retval = 1;
        }
    }
    else if (buf) {
        // format the value we are responsible for
        int max_len = strlen(our_key) + 1 + NET_SSID_MAX_LEN + 1; // key=value\n
        retval = snprintf(buf, max_len, "%s=%s\n", our_key, sys_cfg->wifi_ssid);
    }
    return (retval);
}

int32_t _process_cfg_line(config_t* config, char* line) {
    int32_t retval = 0;
    char* cfgline = strnltonull(strskipws(line));
    char *key;
    char *value;
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
    const cfg_item_handler_fn* handlers = cfg_handlers;
    while (*handlers) {
        cfg_item_handler_fn handler = *handlers;
        retval = handler(config, key, value, NULL);
        if (0 != retval) {
            return (retval);
        }
        handlers++;
    }
    retval = (-1);

    warn_printf("Config - Unknown key: '%s'\n", key);
    return (retval);
}

int32_t _process_sys_cfg_line(char* line) {
    int32_t retval = 0;
    char* cfgline = strnltonull(strskipws(line));
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
    const sys_cfg_item_handler_fn* handlers = sys_cfg_handlers;
    while (*handlers) {
        const sys_cfg_item_handler_fn handler = *handlers;
        retval = handler(&_system_cfg, key, value, NULL);
        if (0 != retval) {
            return (retval);
        }
        handlers++;
    }
    retval = (-1);

    warn_printf("System Config - Unknown key: '%s'\n", key);
    return (retval);
}

const config_t* config_current() {
    return _current_cfg;
}

uint8_t binary_from_bool(int b) {
    return (0 == b ? 0 : 1);
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
            if (FR_OK != fr) {
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
            if (FR_OK != fr) {
                ret = 2;
                error_printf("Config: Could not open file (%d)\r\n", fr);
                break;
            }
            while (f_gets(buf, sizeof(buf), &fil)) {
                _process_cfg_line(config, buf);
            }
            // Save the filename we used
            if (_current_cfg_filename) {
                free(_current_cfg_filename);
            }
            _current_cfg_filename = config_value_create(_system_cfg.user_cfg_filename);
            // Close file
            fr = f_close(&fil);
            if (FR_OK != fr) {
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

bool config_sys_is_set() {
    return (_system_cfg.is_set);
}

char* config_value_create(const char* value) {
    char* malloced_value;
    malloced_value = malloc(strlen(value) + 1);
    strcpy(malloced_value, value);

    return (malloced_value);
}
