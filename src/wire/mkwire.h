/**
 * MorseKOB Wire (network) functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _MUKOB_MKWIRE_H_
#define _MUKOB_MKWIRE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define MKOBSERVER_DEFAULT "mtc-kob.dyndns.org"
#define MKOBSERVER_PORT_DEFAULT 7890

/*!
 * @brief Connect to a MorseKOB Wire.
 * @ingroup wire
 *
 * @param wire_no The wire number to connect to (1-999)
 */
void mkwire_connect(unsigned short wire_no);

/*!
 * @brief Disconnect from the currently connected MorseKOB Wire.
 * @ingroup wire
 */
void mkwire_disconnect();

/*!
 * @brief Initialize the MorseKOB Wire subsystem.
 * @ingroup wire
 *
 * @param mkobs_url MorseKOB Server URL.
 * @param office_id The local Station/Office ID.
 */
void mkwire_init(char *mkobs_url, unsigned short port, char *office_id);

/*!
 * @brief Connected to KOB Server status.
 * @ingroup wire
 * 
 * @returns True if currently connected to a KOB Server wire.
 */
bool mkwire_is_connected();

/*!
 * @brief Set the local Station/Office ID.
 *
 * @param office_id The local Station/Office ID.
 */
void mkwire_set_office_id(char *office_id);

#ifdef __cplusplus
}
#endif
#endif // _MUKOB_MKWIRE_H_
