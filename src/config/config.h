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

typedef struct _config_ {
    uint16_t cfg_version;
    //
    char* wifi_password;
    char* wifi_ssid;
} config_t;

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
 * @brief Allocate memory for a string value and copy the string value into it.
 *
 * @param value The value to allocate for and copy.
 * @return char* The new copy.
 */
extern char* config_value_create(char* value);

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_H_
