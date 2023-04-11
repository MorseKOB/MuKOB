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
#ifndef _KOB_CONFIG_FO_H_
#define _KOB_CONFIG_FO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include "ff.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/types.h"

struct _SYS_CFG_ITEM_HANDLER_CLASS_;
typedef struct _SYS_CFG_ITEM_HANDLER_CLASS_ sys_cfg_item_handler_class_t;

/**
 * @brief System config item handler type. Functions of this type used to process system config file lines.
 * @ingroup config
 *
 * Defines the signature of system config item handlers.
 */
typedef int(*sys_cfg_item_reader_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, config_sys_t* sys_cfg, const char* value);
typedef int(*sys_cfg_item_writer_fn)(const struct _SYS_CFG_ITEM_HANDLER_CLASS_* self, const config_sys_t* sys_cfg, char* buf, bool full);

struct _SYS_CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const char* label;
    const uint16_t id_flag;
    const sys_cfg_item_reader_fn reader;
    const sys_cfg_item_writer_fn writer;
};

struct _CFG_ITEM_HANDLER_CLASS_;
typedef struct _CFG_ITEM_HANDLER_CLASS_ cfg_item_handler_class_t;

/**
 * @brief Config item handler type. Functions of this type used to process config file lines.
 * @ingroup config
 *
 * Defines the signature of config item handlers.
 */
typedef int(*cfg_item_reader_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, config_t* cfg, const char* value);
typedef int(*cfg_item_writer_fn)(const struct _CFG_ITEM_HANDLER_CLASS_* self, const config_t* cfg, char* buf, bool full);

struct _CFG_ITEM_HANDLER_CLASS_ {
    const char* key;
    const char short_opt;
    const char* long_opt;
    const char* label;
    const cfg_item_reader_fn reader;
    const cfg_item_writer_fn writer;
};


// extern void config_fops_module_init(const sys_cfg_item_handler_class_t* (*) sys_cfg_handlers[],
//                                     const cfg_item_handler_class_t* (*) cfg_handlers[]);
extern void config_fops_module_init(const sys_cfg_item_handler_class_t** sys_cfg_handlers, const cfg_item_handler_class_t** cfg_handlers);

/**
 * @brief Read a config file and set the values on a config object.
 * @ingroup config
 *
 * @param cfg The config object to set values on.
 * @param cfg_num The number of the configuration to read.
 * @return F_OK if no errors.
 */
extern FRESULT cfo_read_cfg(config_t* cfg, uint16_t cfg_num);

/**
 * @brief Read from the system config file.
 * @ingroup config
 *
 * @param sys_cfg System config instance to store values into.
 * @return uint16_t Flags for items that could not be set (0 if all could be set).
 */
extern uint16_t cfo_read_sys_cfg(config_sys_t* sys_cfg);

/**
 * @brief Save a configuration.
 *
 * @param cfg The configuration instance to save.
 * @param config_num The number (1-9) to use as the identifier.
 * @return FRESULT File operation result.
 */
extern FRESULT cfo_save_cfg(const config_t* cfg, uint16_t cfg_num);

/**
 * @brief
 *
 * @param sys_cfg The system configuration instance to save.
 * @return FRESULT
 */
extern FRESULT cfo_save_sys_cfg(const config_sys_t* sys_cfg);

#ifdef __cplusplus
}
#endif
#endif // _KOB_CONFIG_FO_H_
