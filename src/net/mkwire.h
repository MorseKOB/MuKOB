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
#include "pico/types.h"

#define MKOBSERVER_DEFAULT "mtc-kob.dyndns.org"
#define MKOBSERVER_PORT_DEFAULT 7890
#define MKOBSERVER_STATION_ID_MAX_LEN 127

typedef enum _WIRE_CONNECTED_STATE_ {
    WIRE_NOT_CONNECTED,
    WIRE_CONNECTED,
} wire_connected_state_t;

/*!
 * @brief Disconnect from the currently connected MorseKOB Wire.
 * @ingroup wire
 */
void mkwire_disconnect();

/*!
 * @brief Connect to a MorseKOB Wire.
 * @ingroup wire
 *
 * @param wire_no The wire number to connect to (1-999)
 */
void mkwire_connect(unsigned short wire_no);

/*!
 * @brief Toggle connection (Connect or Disconnect) to a MorseKOB Wire.
 * @ingroup wire
 *
 * Toggle connection is a common operation. This requires that a wire
 * has been set.
 */
void mkwire_connect_toggle();

/**
 * @brief The wire connected state.
 * @ingroup wire
 *
 * @return wire_connected_state_t The current state
 */
wire_connected_state_t mkwire_connected_state();

/*!
 * @brief Initialize the MorseKOB Wire subsystem.
 * @ingroup wire
 *
 * @param mkobs_url MorseKOB Server URL.
 * @param office_id The local Station/Office ID.
 * @param wire_no The wire number to use for connect.
 */
void mkwire_init(char *mkobs_url, uint16_t port, char *office_id, uint16_t wire_no);

/*!
 * @brief Connected to KOB Server status.
 * @ingroup wire
 *
 * @returns True if currently connected to a KOB Server wire.
 */
bool mkwire_is_connected();

/**
 * @brief Return the host from a Host:Port, or the default.
 * @ingroup wire
 *
 * Copy the host portion of a Host:Port, or the default.
 *
 * @param buf The buffer to copy the host into.
 * @param
 * @param host_and_port A string that is NULL, contains a Host, or a Host:Port
 * @return true It the buffer was large enough to contain the full host.
 * @return false If the complete host couldn't be copied into the buffer.
 */
bool mkwire_host_from_hostport(char* buf, uint32_t maxlen, const char* host_and_port);

/**
 * @brief Return the port from a Host:Port, or the default.
 * @ingroup wire
 *
 * Returns the `port` part of a host:port, or the default MorseKOB
 * Server port if no port is included.
 *
 * @param host_and_port
 * @return uint16_t The port part or the default port.
 */
uint16_t mkwire_port_from_hostport(const char* host_and_port);

/*!
 * @brief Set the local Station/Office ID.
 *
 * @param office_id The local Station/Office ID.
 */
void mkwire_set_office_id(char *office_id);

/**
 * @brief Get the current wire number.
 *
 * @return int16_t The wire number.
 */
uint16_t mkwire_wire_get();

/**
 * @brief Set the wire number (without connecting).
 * @ingroup wire
 *
 * This can also be done with the `mkwire_connect` function,
 * but clients sometimes want to set the wire without connecting.
 *
 * @param wire_no Wire number 1-999 are used/allowed by MorseKOB Server.
 */
void mkwire_wire_set(uint16_t wire_no);

#ifdef __cplusplus
}
#endif
#endif // _MUKOB_MKWIRE_H_
