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

#include "cmd_t.h" // Command processing type definitions
#include "sd_card.h"
#include "string.h"
#include "ff.h"
#include "mkboard.h"
#include "net.h"
#include "util.h"
#include "mkwire.h"
#include "ui_term.h"

#define _CFG_VERSION_KEY "cfg_version"

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
typedef struct _CFG_W_MARKER_ {
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
 *    NOTE: If the value string is to be stored into the config object `str_value_create` should
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
struct _CFG_ITEM_HANDLER_CLASS_;

typedef int(*cfg_item_reader_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
typedef int(*cfg_item_writer_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);

struct _CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const char short_opt;
    const char* long_opt;
    const cfg_item_reader_fn reader;
    const cfg_item_writer_fn writer;
};
typedef struct _CFG_ITEM_HANDLER_CLASS_ cfg_item_handler_class_t;

static int _cih_config_version_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_config_version_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_config_version =
{ _CFG_VERSION_KEY, '\000', NULL, _cih_config_version_reader, _cih_config_version_writer };

static int _cih_auto_connect_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_auto_connect_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_auto_connect =
{ "auto_connect", 'C', "autoconnect", _cih_auto_connect_reader, _cih_auto_connect_writer };

static int _cih_char_speed_min_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_char_speed_min_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_char_speed_min =
{ "char_speed_min", 'c', "charspeed", _cih_char_speed_min_reader, _cih_char_speed_min_writer };

static int _cih_code_type_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_code_type_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_code_type =
{ "code_type", 'T', "type", _cih_code_type_reader, _cih_code_type_writer };

static int _cih_host_port_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_host_port_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_host_port =
{ "server_host_port", 'U', "url", _cih_host_port_reader, _cih_host_port_writer };

static int _cih_key_has_closer_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_key_has_closer_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_key_has_closer =
{ "key_has_closer", 'K', "keycloser", _cih_key_has_closer_reader, _cih_key_has_closer_writer };

static int _cih_key_input_invert_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_key_input_invert_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_key_input_invert =
{ "invert_key_input", 'M', "iki", _cih_key_input_invert_reader, _cih_key_input_invert_writer };

static int _cih_local_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_local_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_local =
{ "local", 'L', "local", _cih_local_reader, _cih_local_writer };

static int _cih_remote_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_remote_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_remote =
{ "remote", 'R', "remote", _cih_remote_reader, _cih_remote_writer };

static int _cih_sound_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_sound_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_sound =
{ "sound", 'a', "sound", _cih_sound_reader, _cih_sound_writer };

static int _cih_sounder_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_sounder_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_sounder =
{ "sounder", 'A', "sounder", _cih_sounder_reader, _cih_sounder_writer };

static int _cih_spacing_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_spacing_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_spacing =
{ "spacing", 's', "spacing", _cih_spacing_reader, _cih_spacing_writer };

static int _cih_station_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_station_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_station =
{ "station", 'S', "station", _cih_station_reader, _cih_station_writer };

static int _cih_text_speed_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_text_speed_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_text_speed =
{ "text_speed", 't', "textspeed", _cih_text_speed_reader, _cih_text_speed_writer };

static int _cih_wire_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
static int _cih_wire_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_wire =
{ "wire", 'W', "wire", _cih_wire_reader, _cih_wire_writer };

/**
 * @brief Array of config item class instances.
 * @ingroup config
 *
 * The entries should be in the order that the config lines should be written to the config file.
 */
static const struct _CFG_ITEM_HANDLER_CLASS_* cfg_handlers[] = {
    & _cihc_config_version,
    & _cihc_auto_connect,
    & _cihc_code_type,
    & _cihc_key_has_closer,
    & _cihc_key_input_invert,
    & _cihc_local,
    & _cihc_char_speed_min,
    & _cihc_remote,
    & _cihc_host_port,
    & _cihc_sound,
    & _cihc_sounder,
    & _cihc_spacing,
    & _cihc_station,
    & _cihc_text_speed,
    & _cihc_wire,
    ((const struct _CFG_ITEM_HANDLER_CLASS_*)0), // NULL last item to signify end
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
 *    NOTE: If the value string is to be stored into the system config object `str_value_create` should
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
struct _SYS_CFG_ITEM_HANDLER_CLASS_;
typedef int(*sys_cfg_item_reader_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
typedef int(*sys_cfg_item_writer_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);

struct _SYS_CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const sys_cfg_item_reader_fn reader;
    const sys_cfg_item_writer_fn writer;
};


static int _scih_config_version_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
static int _scih_config_version_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_config_version =
{ _CFG_VERSION_KEY, _scih_config_version_reader, _scih_config_version_writer };

static int _scih_tz_offset_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
static int _scih_tz_offset_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_tz_offset =
{ "tz_offset", _scih_tz_offset_reader, _scih_tz_offset_writer };

static int _scih_boot_cfg_number_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
static int _scih_boot_cfg_number_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_boot_cfg_number =
{ "bcfg_number", _scih_boot_cfg_number_reader, _scih_boot_cfg_number_writer };

static int _scih_wifi_password_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
static int _scih_wifi_password_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_wifi_password =
{ "wifi_pw", _scih_wifi_password_reader, _scih_wifi_password_writer };

static int _scih_ssid_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value);
static int _scih_ssid_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_ssid =
{ "wifi_ssid", _scih_ssid_reader, _scih_ssid_writer };

static const struct _SYS_CFG_ITEM_HANDLER_CLASS_* sys_cfg_handlers[] = {
    & _scihc_config_version,
    & _scihc_tz_offset,
    & _scihc_boot_cfg_number,
    & _scihc_wifi_password,
    & _scihc_ssid,
    ((const struct _SYS_CFG_ITEM_HANDLER_CLASS_*)0), // NULL last item to signify end
};

static const char* _sys_cfg_filename = "mukob.sys.cfg";
static config_sys_t _system_cfg = { 1, false, 0.0, -1, NULL, NULL };

#define _CFG_BOOT_FILENAME_FORMAT "mukob.%hu.cfg"
static int _current_cfg_number;
static char _boot_cfg_filename[16];
static char _current_cfg_filename[16];
static config_t* _current_cfg;

static int _cih_config_version_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int iv = atoi(value);
    cfg->cfg_version = (uint16_t)iv;

    return (1);
}

static int _cih_config_version_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    // format the value we are responsible for
    int retval = sprintf(buf, "# Config file/format version.\n%s=%hd", self->key, cfg->cfg_version);

    return (retval);
}

static int _cih_auto_connect_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->auto_connect = b;
    retval = 1;

    return (retval);
}

static int _cih_auto_connect_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Autoconnect to wire on startup.\n%s=%hd", self->key, binary_from_bool(cfg->auto_connect));

    return (retval);
}

static const char* _code_type_enum_names[] = {
    "AMERICAN",
    "INTERNATIONAL",
};

static int _cih_code_type_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    for (int i = 0; i < sizeof(_code_type_enum_names); i++) {
        if (strcmp(_code_type_enum_names[i], value) == 0) {
            cfg->code_type = (code_type_t)i;
            retval = 1;
            break;
        }
    }

    return (retval);
}

static int _cih_code_type_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Code type (AMERICAN | INTERNATIONAL).\n%s=%s", self->key, _code_type_enum_names[cfg->code_type]);

    return (retval);
}

static int _cih_key_has_closer_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->key_has_closer = b;
    retval = 1;

    return (retval);
}

static int _cih_key_has_closer_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Does the key have a physical closer.\n%s=%hd", self->key, binary_from_bool(cfg->key_has_closer));

    return (retval);
}

static int _cih_key_input_invert_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->invert_key_input = b;
    retval = 1;

    return (retval);
}

static int _cih_key_input_invert_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Invert the key input (used for modem input).\n%s=%hd", self->key, binary_from_bool(cfg->invert_key_input));

    return (retval);
}

static int _cih_local_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->local = b;
    retval = 1;

    return (retval);
}

static int _cih_local_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Sound key input locally.\n%s=%hd", self->key, binary_from_bool(cfg->local));

    return (retval);
}

static int _cih_char_speed_min_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    int iv = atoi(value);
    cfg->char_speed_min = (uint8_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_char_speed_min_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# The minimum character speed. Used for Farnsworth.\n%s=%hd", self->key, cfg->char_speed_min);

    return (retval);
}

static int _cih_remote_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->remote = b;
    retval = 1;

    return (retval);
}

static int _cih_remote_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Send key input to the remote server.\n%s=%hd", self->key, binary_from_bool(cfg->remote));

    return (retval);
}

static int _cih_host_port_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    if (cfg->host_and_port) {
        free(cfg->host_and_port);
    }
    cfg->host_and_port = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _cih_host_port_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# host:port of MorseKOB Server.\n%s=%s", self->key, cfg->host_and_port);

    return (retval);
}

static int _cih_sound_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->sound = b;
    retval = 1;

    return (retval);
}

static int _cih_sound_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Use the board sound (tone) for code sounding.\n%s=%hd", self->key, binary_from_bool(cfg->sound));

    return (retval);
}

static int _cih_sounder_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    bool b = bool_from_str(value);
    cfg->sounder = b;
    retval = 1;

    return (retval);
}

static int _cih_sounder_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Use the sounder for code sounding.\n%s=%hd", self->key, binary_from_bool(cfg->sounder));

    return (retval);
}

static const char* _spacing_enum_names[] = {
    "NONE",
    "CHAR",
    "WORD",
};

static int _cih_spacing_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    for (int i = 0; i < sizeof(_spacing_enum_names); i++) {
        if (strcmp(_spacing_enum_names[i], value) == 0) {
            cfg->spacing = (code_spacing_t)i;
            retval = 1;
            break;
        }
    }

    return (retval);
}

static int _cih_spacing_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Where to insert space for Farnsworth (NONE | CHAR | WORD).\n%s=%s", self->key, _spacing_enum_names[cfg->spacing]);

    return (retval);
}

static int _cih_station_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    if (cfg->station) {
        free(cfg->station);
    }
    cfg->station = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _cih_station_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Station ID.\n%s=%s", self->key, cfg->station);

    return (retval);
}

static int _cih_text_speed_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    int iv = atoi(value);
    cfg->text_speed = (uint8_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_text_speed_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Text speed (in WPM).\n%s=%hd", self->key, cfg->text_speed);

    return (retval);
}

static int _cih_wire_reader(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value) {
    int retval = 0;

    int iv = atoi(value);
    cfg->wire = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_wire_writer(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# MorseKOB Wire.\n%s=%hd", self->key, cfg->wire);

    return (retval);
}

static int _scih_config_version_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value) {
    int retval = 0;

    int iv = atoi(value);
    _system_cfg.cfg_version = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _scih_config_version_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Config file/format version.\n%s=%hd", self->key, _system_cfg.cfg_version);

    return (retval);
}

static int _scih_tz_offset_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value) {
    int retval = 0;

    float dv = (float)atof(value);
    _system_cfg.tz_offset = dv;
    retval = 1;

    return (retval);
}

static int _scih_tz_offset_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Timezone offset (hours from GMT).\n%s=%.1f", self->key, _system_cfg.tz_offset);

    return (retval);
}

static int _scih_boot_cfg_number_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value) {
    int retval = 0;

    int n = atoi(value);
    if (n > 0 && n < 10) {
        _system_cfg.boot_cfg_number = n;
    }
    else {
        _system_cfg.boot_cfg_number = 0; // Flag as invalid
        error_printf("Config - Invalid value for boot_cfg_number: %s\n", value);
        retval = (-1);
    }

    return (retval);
}

static int _scih_boot_cfg_number_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# Config file to load at boot.\n%s=%hu", self->key, _system_cfg.boot_cfg_number);

    return (retval);
}

static int _scih_wifi_password_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value) {
    int retval = 0;

    if (_system_cfg.wifi_password) {
        free(_system_cfg.wifi_password);
    }
    _system_cfg.wifi_password = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_wifi_password_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# WiFi password.\n%s=%s", self->key, _system_cfg.wifi_password);

    return (retval);
}

static int _scih_ssid_reader(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const char* value) {
    int retval = 0;

    if (_system_cfg.wifi_ssid) {
        free(_system_cfg.wifi_ssid);
    }
    _system_cfg.wifi_ssid = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_ssid_writer(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, char* buf) {
    int retval = 0;

    // format the value we are responsible for
    retval = sprintf(buf, "# WiFi SSID (name)\n%s=%s", self->key, _system_cfg.wifi_ssid);

    return (retval);
}

int _process_cfg_line(config_t* config, char* line) {
    int retval = 0;
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
    const struct _CFG_ITEM_HANDLER_CLASS_** handlers = cfg_handlers;
    while (*handlers) {
        const struct _CFG_ITEM_HANDLER_CLASS_* handler = *handlers;
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

int _process_sys_cfg_line(char* line) {
    int retval = 0;
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
    const struct _SYS_CFG_ITEM_HANDLER_CLASS_** handlers = sys_cfg_handlers;
    while (*handlers) {
        const struct _SYS_CFG_ITEM_HANDLER_CLASS_* handler = *handlers;
        if (strcmp(handler->key, key) == 0) {
            retval = handler->reader(handler, value);
            return (retval);
        }
        handlers++;
    }
    // No handler found for this key
    retval = (-1);

    warn_printf("System Config - Unknown key: '%s'\n", key);
    return (retval);
}

// ============================================================================
// User command processing
// ============================================================================

static int _config_cmd_configure(int argc, char** argv, const char* unparsed) {
    const config_t* cfg = config_current();
    static char buf[512];

    // List the current configuration
    ui_term_printf("Current Config: %d  Boot Config: %d\n", _current_cfg_number, _system_cfg.boot_cfg_number);
    // Run through the handlers and have each list the configuration value...
    const struct _CFG_ITEM_HANDLER_CLASS_** handlers = cfg_handlers;
    while (*handlers) {
        const struct _CFG_ITEM_HANDLER_CLASS_* handler = *handlers;
        int len = sprintf(buf, "(-%c | --%s)  ", handler->short_opt, handler->long_opt);
        handler->writer(handler, cfg, &buf[len-1]);
        ui_term_printf("%s\n", buf);
        handlers++;
    }

    return (0);
}

const cmd_handler_entry_t cmd_cfg_entry = {
    _config_cmd_configure,
    3,
    "cfg",
    "\001configure",
    NULL,
};

const cmd_handler_entry_t cmd_configure_entry = {
    _config_cmd_configure,
    4,
    "configure",
    "[(optname=value | -<flag>/--<longflag> value) [...]]",
    "List current user configuration. Set configuration value(s).",
};


// ============================================================================
// Public
// ============================================================================

const config_t* config_current() {
    return _current_cfg;
}

config_t* config_current_for_modification() {
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
    _system_cfg.boot_cfg_number = -1; // Invalid number as flag
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
                error_printf("Config - Could not mount filesystem (%d)\r\n", fr);
                break;
            }

            // Read the system config first
            fr = f_open(&fil, _sys_cfg_filename, FA_READ);
            if (fr != FR_OK) {
                ret = 2;
                error_printf("Config - Could not open file (%d)\r\n", fr);
                break;
            }
            while (f_gets(buf, sizeof(buf), &fil)) {
                _process_sys_cfg_line(buf);
            }
            // Close file
            fr = f_close(&fil);
            if (FR_OK != fr) {
                ret = 3;
                error_printf("Config - Could not close file (%d)\r\n", fr);
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
            _current_cfg_number = _system_cfg.boot_cfg_number;
            if (_current_cfg_number < 1) {
                _current_cfg_number = 1;
                error_printf("Config - Boot configuration number is not valid. Using '1'.\n");
            }
            sprintf(_boot_cfg_filename, _CFG_BOOT_FILENAME_FORMAT, _current_cfg_number);
            fr = f_open(&fil, _boot_cfg_filename, FA_READ);
            if (FR_OK != fr) {
                ret = 2;
                error_printf("Config - Could not open file '%s' (Error: %d). Trying others...\n", _boot_cfg_filename, fr);
                // Try others...
                int cn = _current_cfg_number;
                _current_cfg_number = -1;
                for (int i = 1; i < 10; i++) {
                    if (i == cn) {
                        continue; // No need to try this one
                    }
                    sprintf(_boot_cfg_filename, _CFG_BOOT_FILENAME_FORMAT, i);
                    fr = f_open(&fil, _boot_cfg_filename, FA_READ);
                    if (FR_OK == fr) {
                        // We can open this one.
                        error_printf(
                            "Config - Using '%s' (instead of '%u')...\n", _boot_cfg_filename, cn
                        );
                        _current_cfg_number = i;
                        break;
                    }
                }
            }
            if (_current_cfg_number < 1) {
                error_printf("Config - Could not find a user config file to use. Using default values.\n");
                break;
            }
            while (f_gets(buf, sizeof(buf), &fil)) {
                _process_cfg_line(config, buf);
            }
            // Save the filename we used
            strcpy(_current_cfg_filename, _boot_cfg_filename);
            // Close file
            fr = f_close(&fil);
            if (FR_OK != fr) {
                ret = 3;
                error_printf("Config - Could not close file '%s' (Error: %d)\n", _boot_cfg_filename, fr);
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
