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

#include "cmd_t.h"

#define CONFIG_NAME_MAX_LEN 15

typedef enum _code_type_ {
    CODE_TYPE_AMERICAN = 0,
    CODE_TYPE_INTERNATIONAL = 1,
} code_type_t;

typedef enum _code_spacing_ {
    CODE_SPACING_NONE = 0,
    CODE_SPACING_CHAR = 1,
    CODE_SPACING_WORD = 2,
} code_spacing_t;

#define CONFIG_NAME_MAX_LEN 15

#define CONFIG_VERSION 1
typedef struct _config_ {
    uint16_t cfg_version;
    //
    char* name;
    bool auto_connect;
    uint8_t char_speed_min;
    code_type_t code_type;
    char* host_and_port; // host/addr:port of the MorseKOB Server
    bool invert_key_input;
    bool key_has_closer;
    bool local;
    bool remote;
    bool sound;
    bool sounder;
    code_spacing_t spacing;
    char* station;
    uint8_t text_speed;
    uint16_t wire;
} config_t;

#define _SYSCFG_VER_ID 0x0001
#define _SYSCFG_BCN_ID 0x0002
#define _SYSCFG_TZ_ID  0x0004
#define _SYSCFG_WP_ID  0x0008
#define _SYSCFG_WS_ID  0x0010
#define _SYSCFG_NOT_LOADED 0x8000

typedef struct _sys_config_ {
    bool is_set;
    //
    uint16_t cfg_version;
    //
    uint8_t boot_cfg_number;
    float tz_offset;
    char* wifi_password;
    char* wifi_ssid;
} config_sys_t;

extern const cmd_handler_entry_t cmd_bootcfg_entry;
extern const cmd_handler_entry_t cmd_cfg_entry;
extern const cmd_handler_entry_t cmd_configure_entry;
extern const cmd_handler_entry_t cmd_load_entry;
extern const cmd_handler_entry_t cmd_save_entry;
extern const cmd_handler_entry_t cmd_station_entry;

/**
 * @brief Clear the values of a configuration instance to the initial values.
 * @ingroup config
 *
 * @param cfg The config instance to clear the values from.
 * @return config_t* Pointer to the destination config instance passed in (for convenience)
 */
extern config_t* config_clear(config_t* cfg);

/**
 * @brief Copy values from one config instance to another.
 * @ingroup config
 *
 * @param cfg_dest An existing config instance to copy values into.
 * @param cfg_source Config instance to copy values from.
 * @return config_t* Pointer to the destination config instance passed in (for convenience)
 */
extern config_t* config_copy(config_t* cfg_dest, const config_t* cfg_source);

/**
 * @brief Get the current configuration.
 * @ingroup config
 *
 * @return config_t* Current configuration.
 */
extern const config_t* config_current();

/**
 * @brief Get the current configuration to be modified.
 *
 * @return config_t* Current configuration.
 */
extern config_t* config_current_for_modification();

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
 * @brief Post a message to both cores indicating that the configuration has changed.
 */
extern void config_indicate_changed();

/**
 * @brief Initialize the configuration subsystem
 * @ingroup config
*/
extern int config_module_init();

/**
 * @brief Load the current config from saved config number.
 * @ingroup config
 *
 * @param config_num The config number to load.
 * @return true Config was loaded.
 */
extern bool config_load(int config_num);

/**
 * @brief Allocate a new config_t structure. Optionally, initialize values.
 * @ingroup config
 *
 * @see config_free(config_t* config)
 * @param init_values Config structure with initial values, or NULL for an empty config.
 * @return config_t* A newly allocated config_t structure. Must be free'ed with `config_free()`.
 */
extern config_t* config_new(const config_t* init_values);

/**
 * @brief Save the current configuration. Optionally set this as the boot configuration.
 * @ingroup config
 *
 * @param config_num The number to assign to the configuration. Must be 1-9.
 * @param set_as_boot True to set this configuration as the boot configuration.
 * @return bool True if successful.
 */
extern bool config_save(int config_num, bool set_as_boot);

/**
 * @brief Set the configuration number as the boot configuration.
 *
 * @param config_num The configuration number to set as boot. Must be 1-9.
 * @return bool True if successful.
 */
extern bool config_set_boot(int config_num);

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

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_H_
