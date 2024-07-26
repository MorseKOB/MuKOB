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
#include "config_fops.h"

#include "mkboard.h"
#include "mkwire.h"
#include "net.h"
#include "ui_term.h"
#include "util.h"

#include "string.h"

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

static int _cih_config_version_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);

static int _cih_auto_connect_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_auto_connect_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_auto_connect =
{ "auto_connect", 'C', "autoconnect", "Auto connect", _cih_auto_connect_reader, _cih_auto_connect_writer };

static int _cih_char_speed_min_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_char_speed_min_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_char_speed_min =
{ "char_speed_min", 'c', "charspeed", "Character speed (WPM)", _cih_char_speed_min_reader, _cih_char_speed_min_writer };

static int _cih_code_type_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_code_type_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_code_type =
{ "code_type", 'T', "type", "Code type", _cih_code_type_reader, _cih_code_type_writer };

static int _cih_host_port_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_host_port_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_host_port =
{ "server_host_port", 'U', "url", "Morse KOB Server host:port", _cih_host_port_reader, _cih_host_port_writer };

static int _cih_key_has_closer_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_key_has_closer_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_key_has_closer =
{ "key_has_closer", 'K', "keycloser", "Key has closer", _cih_key_has_closer_reader, _cih_key_has_closer_writer };

static int _cih_key_input_invert_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_key_input_invert_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_key_input_invert =
{ "invert_key_input", 'M', "iki", "Invert key input", _cih_key_input_invert_reader, _cih_key_input_invert_writer };

static int _cih_local_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_local_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_local =
{ "local", 'L', "local", "Sound and copy local code", _cih_local_reader, _cih_local_writer };

static int _cih_name_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);

static int _cih_remote_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_remote_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_remote =
{ "remote", 'R', "remote", "Send to wire", _cih_remote_reader, _cih_remote_writer };

static int _cih_sound_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_sound_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_sound =
{ "sound", 'a', "sound", "Use audio (tone)", _cih_sound_reader, _cih_sound_writer };

static int _cih_sounder_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_sounder_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_sounder =
{ "sounder", 'A', "sounder", "Use sounder", _cih_sounder_reader, _cih_sounder_writer };

static int _cih_spacing_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_spacing_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_spacing =
{ "spacing", 's', "spacing", "Where to add spacing for Farnsworth", _cih_spacing_reader, _cih_spacing_writer };

static int _cih_station_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_station_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_station =
{ "station", 'S', "station", "Our station name", _cih_station_reader, _cih_station_writer };

static int _cih_text_speed_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_text_speed_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_text_speed =
{ "text_speed", 't', "textspeed", "The text/overall speed (WPM)", _cih_text_speed_reader, _cih_text_speed_writer };

static int _cih_wire_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value);
static int _cih_wire_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full);
static struct _CFG_ITEM_HANDLER_CLASS_ _cihc_wire =
{ "wire", 'W', "wire", "MorseKOB Server wire to connect to", _cih_wire_reader, _cih_wire_writer };

/**
 * @brief Array of config item class instances.
 * @ingroup config
 *
 * The entries should be in the order that the config lines should be written to the config file.
 */
static const cfg_item_handler_class_t* _cfg_handlers[] = {
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
    ((const cfg_item_handler_class_t*)0), // NULL last item to signify end
};

static int _scih_tz_offset_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_tz_offset_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_tz_offset =
{ "tz_offset", "Time zone offset (hours from GMT)", _SYSCFG_TZ_ID, _scih_tz_offset_reader, _scih_tz_offset_writer };

static int _scih_boot_cfg_number_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_boot_cfg_number_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_boot_cfg_number =
{ "bcfg_number", "Config number to load at boot", _SYSCFG_BCN_ID, _scih_boot_cfg_number_reader, _scih_boot_cfg_number_writer };

static int _scih_disp_wrap_back_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_disp_wrap_back_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_disp_wrap_back =
{ "disp_wrap_back", "Display text characters to scan back for EOL wrap", _SYSCFG_DWB_ID, _scih_disp_wrap_back_reader, _scih_disp_wrap_back_writer};

static int _scih_wifi_password_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_wifi_password_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_wifi_password =
{ "wifi_pw", "Wi-Fi password", _SYSCFG_WP_ID, _scih_wifi_password_reader, _scih_wifi_password_writer };

static int _scih_ssid_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value);
static int _scih_ssid_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full);
static const struct _SYS_CFG_ITEM_HANDLER_CLASS_ _scihc_ssid =
{ "wifi_ssid", "Wi-Fi SSID (name)", _SYSCFG_WS_ID, _scih_ssid_reader, _scih_ssid_writer };

static const sys_cfg_item_handler_class_t* _sys_cfg_handlers[] = {
    & _scihc_tz_offset,
    & _scihc_boot_cfg_number,
    & _scihc_wifi_password,
    & _scihc_ssid,
    & _scihc_disp_wrap_back,
    ((const sys_cfg_item_handler_class_t*)0), // NULL last item to signify end
};

static config_sys_t _system_cfg = { 1, false, 0.0, -1, NULL, NULL };
static uint16_t _sys_not_init_flags;

static int _current_cfg_number;
static config_t* _current_cfg;

static cmt_msg_t _msg_config_changed = { MSG_CONFIG_CHANGED, {0} };

static int _cih_auto_connect_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->auto_connect = b;
    retval = 1;

    return (retval);
}

static int _cih_auto_connect_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Autoconnect to wire on startup.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->auto_connect));

    return (len);
}

static const char* _code_type_enum_names[] = {
    "AMERICAN",
    "INTERNATIONAL",
};

static int _cih_code_type_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    for (int i = 0; i < sizeof(_code_type_enum_names); i++) {
        if (strcmp(_code_type_enum_names[i], value) == 0) {
            cfg->code_type = (code_type_t)i;
            retval = 1;
            break;
        }
    }

    return (retval);
}

static int _cih_code_type_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Code type (AMERICAN | INTERNATIONAL).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", _code_type_enum_names[cfg->code_type]);

    return (len);
}

static int _cih_key_has_closer_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->key_has_closer = b;
    retval = 1;

    return (retval);
}

static int _cih_key_has_closer_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Does the key have a physical closer.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->key_has_closer));

    return (len);
}

static int _cih_key_input_invert_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->invert_key_input = b;
    retval = 1;

    return (retval);
}

static int _cih_key_input_invert_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Invert the key input (used for modem input).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->invert_key_input));

    return (len);
}

static int _cih_local_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->local = b;
    retval = 1;

    return (retval);
}

static int _cih_local_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Sound key input locally.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->local));

    return (len);
}

static int _cih_char_speed_min_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    int iv = atoi(value);
    cfg->char_speed_min = (uint8_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_char_speed_min_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# The minimum character speed. Used for Farnsworth.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", cfg->char_speed_min);

    return (len);
}

static int _cih_remote_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->remote = b;
    retval = 1;

    return (retval);
}

static int _cih_remote_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Send key input to the remote server.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->remote));

    return (len);
}

static int _cih_host_port_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    if (cfg->host_and_port) {
        free(cfg->host_and_port);
    }
    cfg->host_and_port = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _cih_host_port_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# host:port of MorseKOB Server.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", cfg->host_and_port);

    return (len);
}

static int _cih_sound_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->sound = b;
    retval = 1;

    return (retval);
}

static int _cih_sound_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Use the board sound (tone) for code sounding.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->sound));

    return (len);
}

static int _cih_sounder_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    bool b = bool_from_str(value);
    cfg->sounder = b;
    retval = 1;

    return (retval);
}

static int _cih_sounder_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Use the sounder for code sounding.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", binary_from_int(cfg->sounder));

    return (len);
}

static const char* _spacing_enum_names[] = {
    "NONE",
    "CHAR",
    "WORD",
};

static int _cih_spacing_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    for (int i = 0; i < sizeof(_spacing_enum_names); i++) {
        if (strcmp(_spacing_enum_names[i], value) == 0) {
            cfg->spacing = (code_spacing_t)i;
            retval = 1;
            break;
        }
    }

    return (retval);
}

static int _cih_spacing_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Where to insert space for Farnsworth (NONE | CHAR | WORD).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", _spacing_enum_names[cfg->spacing]);

    return (len);
}

static int _cih_station_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    if (cfg->station) {
        free(cfg->station);
    }
    cfg->station = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _cih_station_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Station ID.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", cfg->station);

    return (len);
}

static int _cih_text_speed_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = 0;

    int iv = atoi(value);
    cfg->text_speed = (uint8_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_text_speed_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Text speed (in WPM).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", cfg->text_speed);

    return (len);
}

static int _cih_wire_reader(const cfg_item_handler_class_t* self, config_t* cfg, const char* value) {
    int retval = -1;

    int iv = atoi(value);
    cfg->wire = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _cih_wire_writer(const cfg_item_handler_class_t* self, const config_t* cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# MorseKOB Wire.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", cfg->wire);

    return (len);
}

static int _scih_tz_offset_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    float dv = (float)atof(value);
    sys_cfg->tz_offset = dv;
    retval = 1;

    return (retval);
}

static int _scih_tz_offset_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Timezone offset (hours from GMT).\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%.1f", sys_cfg->tz_offset);

    return (len);
}

static int _scih_boot_cfg_number_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    int n = atoi(value);
    if (n > 0 && n < 10) {
        sys_cfg->boot_cfg_number = n;
    }
    else {
        sys_cfg->boot_cfg_number = 0; // Flag as invalid
        error_printf(false, "Config - Invalid value for boot_cfg_number: %s\n", value);
        retval = (-1);
    }

    return (retval);
}

static int _scih_boot_cfg_number_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Config file to load at boot.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hu", sys_cfg->boot_cfg_number);

    return (len);
}

static int _scih_disp_wrap_back_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    int iv = atoi(value);
    sys_cfg->disp_wrap_back = (uint16_t)iv;
    retval = 1;

    return (retval);
}

static int _scih_disp_wrap_back_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# Display characters to scan back from EOL for NL wrapping.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%hd", sys_cfg->disp_wrap_back);

    return (len);
}

static int _scih_wifi_password_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    if (sys_cfg->wifi_password) {
        free(sys_cfg->wifi_password);
    }
    sys_cfg->wifi_password = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_wifi_password_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# WiFi password.\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", sys_cfg->wifi_password);

    return (len);
}

static int _scih_ssid_reader(const sys_cfg_item_handler_class_t* self, config_sys_t* sys_cfg, const char* value) {
    int retval = -1;

    if (sys_cfg->wifi_ssid) {
        free(sys_cfg->wifi_ssid);
    }
    sys_cfg->wifi_ssid = str_value_create(value);
    retval = 1;

    return (retval);
}

static int _scih_ssid_writer(const sys_cfg_item_handler_class_t* self, const config_sys_t* sys_cfg, char* buf, bool full) {
    int len = 0;

    // If full - print comment and key
    if (full) {
        len = sprintf(buf, "# WiFi SSID (name)\n%s=", self->key);
    }
    // format the value we are responsible for
    len += sprintf(buf + len, "%s", sys_cfg->wifi_ssid);

    return (len);
}

// ============================================================================
// User command processing
// ============================================================================

static int _config_cmd_bootcfg(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = _current_cfg_number;

    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
    }
    retval = config_set_boot(cfg_num);

    return (retval);
}

static int _config_cmd_configure(int argc, char** argv, const char* unparsed) {
    config_t* newcfg = NULL;
    static char buf[512];
    const cfg_item_handler_class_t** handlers = _cfg_handlers;
    int retval = -1;

    // If there are parameters, try to set values
    if (argc > 1) {
        // Create a config put set the values into. That way, if there is an error the
        // config in use is left unchanged.
        newcfg = config_new(_current_cfg);
        int argn = 1;
        while (argn < argc) {
            char* arg = argv[argn++];
            char dash = '\000';
            char* dashdash = NULL;
            char* key = NULL;
            char* value = NULL;
            const char* eq = "=";
            handlers = _cfg_handlers;
            const cfg_item_handler_class_t* handler = NULL;
            // First, check for our -h/--help
            if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0) {
                // Yes - print the options for setting and exit.
                ui_term_printf("Options to set configuration values:\n");
                while (*handlers) {
                    const cfg_item_handler_class_t* handler = *handlers;
                    bool has_dash = ('\040' < handler->short_opt);
                    bool has_dashdash = (*handler->long_opt);
                    // If there aren't any options, the value isn't settable using this command.
                    if (!has_dash && !has_dashdash) {
                        handlers++;
                        continue;
                    }
                    // Get the description from the handler class
                    int len = sprintf(buf, "%s:\n  ", handler->label);
                    // Format the dash option string
                    if (has_dash && has_dashdash) {
                        len += sprintf(buf + len, "-%c value, --%s value, ", handler->short_opt, handler->long_opt);
                    }
                    else if (has_dash && !has_dashdash) {
                        len += sprintf(buf + len, "-%c value, ", handler->short_opt);
                    }
                    else if (!has_dash && has_dashdash) {
                        len += sprintf(buf + len, "--%s value, ", handler->long_opt);
                    }
                    // Add the key=value
                    len += sprintf(buf + len, "%s=value", handler->key);
                    ui_term_printf("%s\n", buf);
                    handlers++;
                }
                goto help_exit;
            }
            // Find the handler for this option/key
            if (*arg == '-') {
                if (*(arg+1) != '-') {
                    // -opt
                    dash = *(arg + 1);
                }
                else {
                    // --long_opt
                    dashdash = arg + 2;
                }
            }
            else if (strchr(arg, *eq)) {
                // key=value
                key = strtok_r(arg, eq, &arg);
                if (NULL != key) {
                    value = strtok_r(arg, eq, &arg);
                }
            }
            else {
                // No opt, long opt, or key=value
                ui_term_printf(" Invalid input: `%s`\n", arg);
                goto error_exit;
            }
            while (*handlers) {
                handler = *handlers;
                if (key) {
                    if (strcmp(handler->key, key) == 0) {
                        break;
                    }
                }
                else if (dashdash) {
                    if (strcmp(handler->long_opt, dashdash) == 0) {
                        break;
                    }
                }
                else if (handler->short_opt == dash) {
                    break;
                }
                // That handler didn't match. Try the next...
                handler = NULL;
                handlers++;
            }
            if (!handler) {
                // Had an error
                char* reason = (key ? "key" : "option");
                char* errval = (key ? key : (dashdash ? dashdash : (arg+1)));
                ui_term_printf(" Invalid %s: `%s`\n", reason, errval);
                goto error_exit;
            }
            // if we don't have a value we must have had a -opt or --opt, read the next param for the value.
            char* errkey = (key ? key : arg);
            if (!value) {
                if (argn < argc) {
                    value = argv[argn++];
                }
                else {
                    ui_term_printf(" Missing value for `%s`\n", errkey);
                    goto error_exit;
                }
            }
            retval = handler->reader(handler, newcfg, value);
            handler = NULL; // Clear the handler to move to the next argument (if any)
            if (retval < 0) {
                ui_term_printf(" Invalid value for `%s`: `%s`\n", errkey, value);
                goto error_exit;
            }
        }
        // At this point we've processed all of the supplied arguments. Set the config
        config_free(_current_cfg);
        _current_cfg = newcfg;
        newcfg = NULL;
        // Indicate that the config has changed.
        config_indicate_changed();
    }

    // List the current configuration
    retval = 0; // If we get here, we can return 'OK'
    ui_term_printf("Current Config: %s (%d)  Boot Config: %d\n", _current_cfg->name ,_current_cfg_number, _system_cfg.boot_cfg_number);
    // Run through the handlers and have each list the configuration value...
    handlers = _cfg_handlers;
    // Find the longest label
    int max_lbl_len = 0;
    while (*handlers) {
        const cfg_item_handler_class_t* handler = *handlers;
        int lbl_len = strlen(handler->label);
        if (lbl_len > max_lbl_len) {
            max_lbl_len = lbl_len;
        }
        handlers++;
    }
    max_lbl_len += 2; // Add indent
    handlers = _cfg_handlers;
    while (*handlers) {
        const cfg_item_handler_class_t* handler = *handlers;
        int len = 0;
        handler->writer(handler, _current_cfg, buf + len, false);
        ui_term_printf("%*s: %s\n", max_lbl_len, handler->label, buf);
        handlers++;
    }

help_exit:
error_exit: // Free the `newcfg` if one was created.
    config_free(newcfg);

    return (retval);
}

static int _config_cmd_load(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = _current_cfg_number;

    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
    }
    retval = config_load(cfg_num);
    if (retval) {
        ui_term_printf("Loaded - %d:%s\n", cfg_num, _current_cfg->name);
        config_indicate_changed();
    }

    return (retval);
}

static int _config_cmd_save(int argc, char** argv, const char* unparsed) {
    int retval = 0;
    int cfg_num = _current_cfg_number;
    config_t* cfg = config_new(_current_cfg);

    if (argc > 3) {
        ui_term_printf("Too many parameters.\n");
        cmd_help_display(&cmd_save_entry, HELP_DISP_USAGE);
        return (-1);
    }
    if (argc > 1) {
        // The first arg is the number to use or a '.' (meaning current)
        if (strcmp(".", argv[1]) != 0) {
            cfg_num = atoi(argv[1]);
            if (cfg_num < 1 || cfg_num > 9) {
                ui_term_printf("Configuration number must be 1-9\n");
                return (-1);
            }
        }
        if (argc > 2) {
            // The config name. Check for valid characters.
            const char* name = argv[2];
            if (strcmp(cfg->name, name) != 0) {
                int namelen = strlen(name);
                if (namelen > CONFIG_NAME_MAX_LEN) {
                    ui_term_printf("Name can be a maximum of %d characters long.\n", CONFIG_NAME_MAX_LEN);
                    return (-2);
                }
                for (int i = 0; i < namelen; i++) {
                    char c = *(name + i);
                    if (!(isalnum(c) || '_' == c || '-' == c || '.' == c)) {
                        ui_term_printf("Name can only contain letters, numbers, period, dash, and underscore.\n");
                        return (-3);
                    }
                }
                if (cfg->name) {
                    free(cfg->name);
                }
                cfg->name = str_value_create(name);
            }
        }
    }
    FRESULT fr = cfo_save_cfg(cfg, cfg_num);
    if (FR_OK != fr) {
        ui_term_printf("Error saving config.");
        return (-1);
    }
    config_free(_current_cfg);
    _current_cfg = cfg;
    ui_term_printf("Saved config %d (%s)\n", cfg_num, _current_cfg->name);
    config_indicate_changed();

    return (retval);
}

static int _config_cmd_station(int argc, char** argv, const char* unparsed) {
    config_t* cfg = config_current_for_modification();
    if (argc > 1) {
        // Set the station name from the unparsed command line.
        const char* station_name = strskipws(unparsed + strlen(argv[0]));
        if (strcmp(station_name, cfg->station) != 0) {
            cfg->station = str_value_create(station_name);
            ui_term_printf("Station set to: %s\n", cfg->station);
            config_indicate_changed();
        }
    }
    else {
        ui_term_printf("%s\n", cfg->station);
    }

    return (0);
}


const cmd_handler_entry_t cmd_bootcfg_entry = {
    _config_cmd_bootcfg,
    2,
    "bootcfg",
    "[number|.]",
    "Set the current or a specific configuration as the startup.",
};

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

const cmd_handler_entry_t cmd_load_entry = {
    _config_cmd_load,
    2,
    "load",
    "[(number|.)]",
    "Reload the current config. Load a specific config.",
};

const cmd_handler_entry_t cmd_save_entry = {
    _config_cmd_save,
    2,
    "save",
    "[(number|.) [name]]",
    "Save the current config. Save as number (1-9). Save and name.",
};

const cmd_handler_entry_t cmd_station_entry = {
    _config_cmd_station,
    2,
    "station",
    "[station_name]",
    "List the current station name. Set the station name.",
};


// ============================================================================
// Public
// ============================================================================

extern config_t* config_clear(config_t* cfg) {
    if (cfg) {
        cfg->cfg_version = CONFIG_VERSION;
        if (cfg->name) {
            free(cfg->name);
            cfg->name = NULL;
        }
        cfg->auto_connect = false;
        cfg->char_speed_min = 20;
        cfg->code_type = CODE_TYPE_AMERICAN;
        if (cfg->host_and_port) {
            free(cfg->host_and_port);
            cfg->host_and_port = NULL;
        }
        cfg->invert_key_input = false;
        cfg->key_has_closer = false;
        cfg->local = false;
        cfg->remote = false;
        cfg->sound = false;
        cfg->sounder = false;
        cfg->spacing = CODE_SPACING_NONE;
        if (cfg->station) {
            free(cfg->station);
            cfg->station = NULL;
        }
        cfg->text_speed = 20;
        cfg->wire = 101; // MTC Info
    }
    return (cfg);
}

config_t* config_copy(config_t* cfg_dest, const config_t* cfg_source) {
    if (cfg_dest && cfg_source) {
        config_clear(cfg_dest); // Assure that alloc'ed values are freed
        cfg_dest->cfg_version = cfg_source->cfg_version;
        cfg_dest->name = str_value_create(cfg_source->name);
        cfg_dest->auto_connect = cfg_source->auto_connect;
        cfg_dest->char_speed_min = cfg_source->char_speed_min;
        cfg_dest->code_type = cfg_source->code_type;
        cfg_dest->host_and_port = str_value_create(cfg_source->host_and_port);
        cfg_dest->invert_key_input = cfg_source->invert_key_input;
        cfg_dest->key_has_closer = cfg_source->key_has_closer;
        cfg_dest->local = cfg_source->local;
        cfg_dest->remote = cfg_source->remote;
        cfg_dest->sound = cfg_source->sound;
        cfg_dest->sounder = cfg_source->sounder;
        cfg_dest->spacing = cfg_source->spacing;
        cfg_dest->station = str_value_create(cfg_source->station);
        cfg_dest->text_speed = cfg_source->text_speed;
        cfg_dest->wire = cfg_source->wire;
    }
    return (cfg_dest);
}

const config_t* config_current() {
    return _current_cfg;
}

config_t* config_current_for_modification() {
    return _current_cfg;
}

void config_free(config_t* cfg) {
    if (cfg) {
        // If this is a config object there is a marker one byte before the beginning.
        // This is to keep it from accidentally being freed directly by `free`, as there
        // are contained structures that also need to be freed.
        _cfg_w_marker_t* cfgwm = (_cfg_w_marker_t*)((uint8_t*)cfg - (sizeof(_cfg_w_marker_t) - sizeof(config_t)));
        if (cfgwm->marker == _CFG_MEM_MARKER_) {
            // Okay, we can free things up...
            // First, free allocated values
            config_clear(cfg);
            // Now free up the main config structure
            free(cfgwm);
        }
    }
}

void config_indicate_changed() {
    postBothMsgBlocking(&_msg_config_changed);
}

extern bool config_load(int config_num) {
    FRESULT fr;

    config_t* cfg = config_new(NULL);
    fr = cfo_read_cfg(cfg, config_num);
    if (FR_OK != fr) {
        ui_term_printf("Could not load configuration %d. (%d)\n", config_num, fr);
        config_free(cfg);
        return (false);
    }
    config_copy(_current_cfg, cfg);
    _current_cfg_number = config_num;
    config_free(cfg);
    config_indicate_changed();

    return (true);
}

config_t* config_new(const config_t* init_values) {
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
            cfg->auto_connect = init_values->auto_connect;
            cfg->char_speed_min = init_values->char_speed_min;
            cfg->code_type = init_values->code_type;
            cfg->host_and_port = str_value_create(init_values->host_and_port);
            cfg->invert_key_input = init_values->invert_key_input;
            cfg->key_has_closer = init_values->key_has_closer;
            cfg->local = init_values->local;
            cfg->name = str_value_create(init_values->name);
            cfg->remote = init_values->remote;
            cfg->sound = init_values->sound;
            cfg->sounder = init_values->sounder;
            cfg->spacing = init_values->spacing;
            cfg->station = str_value_create(init_values->station);
            cfg->text_speed = init_values->text_speed;
            cfg->wire = init_values->wire;
        }
    }

    return (cfg);
}

extern bool config_save(int config_num, bool set_as_boot) {
    FRESULT fr;

    fr = cfo_save_cfg(_current_cfg, config_num);
    if (FR_OK != fr) {
        ui_term_printf("Could not save configuration %d. (%d)\n", config_num, fr);
        return (false);
    }
    _current_cfg_number = config_num;

    if (set_as_boot) {
        _system_cfg.boot_cfg_number = config_num;
        cfo_save_sys_cfg(&_system_cfg);
    }

    return (true);
}

const config_sys_t* config_sys() {
    return (&_system_cfg);
}

bool config_sys_is_set() {
    return (_system_cfg.is_set);
}

extern bool config_set_boot(int config_num) {
    bool success = false;

    if (config_num >0 && config_num < 10) {
        int cn = _system_cfg.boot_cfg_number;
        _system_cfg.boot_cfg_number = config_num;
        FRESULT fr = cfo_save_sys_cfg(&_system_cfg);
        if (FR_OK != fr) {
            _system_cfg.boot_cfg_number = cn;
            success = false;
        }
    }
    return (success);
}


// ============================================================================
// Initialization
// ============================================================================

int config_module_init() {
    // Set default values in the system config
    _system_cfg.cfg_version = CONFIG_VERSION;
    _system_cfg.boot_cfg_number = -1; // Invalid number as flag
    _system_cfg.wifi_ssid = NULL;
    _system_cfg.wifi_password = NULL;
    // Create a config object to use as the current
    config_t* cfg = config_new(NULL);
    _current_cfg = cfg;
    cfg->cfg_version = CONFIG_VERSION;

    // Initialize the file operations module
    config_fops_module_init(_sys_cfg_handlers, _cfg_handlers);

    FRESULT fr = FR_OK;
    // See if we can read the system and user config from the '.cfg' files...
    _sys_not_init_flags = cfo_read_sys_cfg(&_system_cfg);
    if (_sys_not_init_flags) {
        // Something wasn't initialized. See if we got the config number.
        if (_sys_not_init_flags & _SYSCFG_NOT_LOADED) {
            error_printf(false, "Config - Unable to load system configuration.\n");
            return (FR_DISK_ERR);
        }
        if (_sys_not_init_flags & _SYSCFG_BCN_ID) {
            // Nope... Default to #1
            error_printf(false, "Config - Boot configuration number is not valid. Using '1'.\n");
            _system_cfg.boot_cfg_number = 1;
        }
    }
    _current_cfg_number = _system_cfg.boot_cfg_number;
    fr = cfo_read_cfg(cfg, _current_cfg_number);
    if (fr != FR_OK) {
        error_printf(false, "Config - Could not load configuration (#%hu).\n", _current_cfg_number);
    }

    return (fr);
}
