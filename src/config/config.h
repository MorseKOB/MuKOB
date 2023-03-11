/**
 * KOB Configuration functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _KOB_CONFIG_H_
#define _KOB_CONFIG_H_
#ifdef __cplusplus
 extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>
#include "pico/types.h"

typedef enum _code_type_ {
    CODE_TYPE_AMERICAN = 0,
    CODE_TYPE_INTERNATIONAL = 1,
} code_type_t;

typedef enum _code_spacing_ {
    CODE_SPACING_NONE = 0,
    CODE_SPACING_CHAR = 1,
    CODE_SPACING_WORD = 2,
} code_spacing_t;

#define CONFIG_VERSION 1
typedef struct _config_ {
    uint16_t cfg_version;
    //
    bool auto_connect;
    code_type_t code_type;
    bool key_has_closer;
    bool local;
    uint8_t char_speed_min;
    bool remote;
    char* server_url; // server:port
    bool sound;
    bool sounder;
    code_spacing_t spacing;
    char* station;
    uint8_t text_speed;
    uint16_t wire;
} config_t;

typedef struct _sys_config_ {
    uint16_t cfg_version;
    bool is_set;
    //
    double tz_offset;
    char* user_cfg_filename;
    char* wifi_password;
    char* wifi_ssid;
} config_sys_t;

/**
 * @brief Get the current configuration.
 * @ingroup config
 *
 * @return config_t* Current configuration.
 */
extern const config_t* config_current();

/**
 * @brief Free a config_t structure previously allowcated with config_new.
 * @ingroup config
 *
 * @see config_new(config_t*)
 *
 * @param config Pointer to the config_t structure to free.
 */
extern void config_free(config_t* config);

/**
 * @brief Initialize the configuration subsystem
 * @ingroup config
*/
extern int config_init(void);

/**
 * @brief Allocate a new config_t structure. Optionally, initialize values.
 * @ingroup config
 *
 * @see config_free(config_t* config)
 * @param init_values Config structure with initial values, or NULL for an empty config.
 * @return config_t* A newly allocated config_t structure. Must be free'ed with `config_free()`.
 */
extern config_t* config_new(config_t* init_values);

/**
 * @brief Get the system configuration.
 * @ingroup config
 *
 * @return const config_sys_t* The system configuration.
 */
extern const config_sys_t* config_sys();

/**
 * @brief Indicates if the system config was read and set
 * @ingroup config
 *
 * @return true System config is available.
 * @return false System config could not be read and isn't valid.
 */
extern bool config_sys_is_set();

/**
 * @brief Allocate memory for a string value and copy the string value into it.
 * @ingroup config
 *
 * @param value The value to allocate for and copy.
 * @return char* The new copy.
 */
extern char* config_value_create(const char* value);

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_H_
