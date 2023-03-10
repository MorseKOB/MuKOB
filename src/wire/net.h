/**
 * MuKOB Network functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _KOB_NET_H_
#define _KOB_NET_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include "system_defs.h"
#include "lwip/err.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define NET_SSID_MAX_LEN 32
#define NET_PASSWORD_MAX_LEN 128

/**
 * @brief Function prototype for UDP Bind response handler.
 * @ingroup wire
 *
 * @param status The status from the operation.
 * @param udp_pcb The udp_pcb that was bound, or NULL if an error occurred.
 */
typedef void (*udp_bind_handler_fn)(err_enum_t status, struct udp_pcb* udp_pcb);

/**
 * @brief Function prototype for UDP single operation result handler.
 * @ingroup wire
 *
 * @param status The status from the operation.
 * @param p The PBUF from the UDP operation. It is the handler's responsibility to free the pbuf.
 */
typedef void (*udp_sop_result_handler_fn)(err_enum_t status, struct pbuf* p, void* handler_data);

/**
 * @brief Connect to WiFi (if needed).
 * @ingroup wire
 *
 * @returns true if connected, false if failed to connect.
 */
bool wifi_connect();

/**
 * @brief Status of WiFi connection.
 * @ingroup wire
 *
 * This returns the stored state. It does not try to connect.
 *
 * @returns true if connected.
 */
bool wifi_connected();

/**
 * @brief Set the ssid and password for the WiFi connection.
 * @ingroup wire
 *
 * @param ssid WiFi name.
 * @param pw WiFi password.
 */
void wifi_set_creds(const char* ssid, const char* pw);

/**
 * @brief Make a NTP network call and use the result to update the board's RTC.
 * @ingroup wire
 *
 * @param tz_offset Hours offset from UTC. For a timezone like UTC−09:30, use a value of -9.5
 * @returns ERR_OK (0) on success
 */
err_enum_t network_update_rtc(double tz_offset);

/**
 * @brief Send a UDP message and process the response message.
 * @ingroup wire
 *
 * @param hostname The fully qualified name of the host. This will be used to do a DNS lookup to obtain an IP address.
 * @param port A port number to use when sending the request.
 * @param bind_handler A function to be called after the hostname is resolved and a UDP socket is bound.
 *
 * @returns Error number (from err.h). ERR_OK or ERR_INPROGRESS is returned on success.
 */
err_enum_t udp_socket_bind(const char* hostname, uint16_t port, udp_bind_handler_fn bind_handler);

/**
 * @brief Perform a single UDP operation, consisting of sending a message and getting a response message.
 * @ingroup wire
 *
 * @param hostname The fully qualified name of the host. This will be used to do a DNS lookup to obtain an IP address.
 * @param port A port number to use when sending the request.
 * @param p A PBUF structure containing the message to be sent.
 * @param timeout A millisecond timeout value to use for the request/response. If a timeout occurs, the result handler
 *          will be called with a status of ERR_TOUT and a NULL PBUF.
 * @param result_handler Function called with the result when it is received.
 * @param handler_data Data that is passed to the result handler along with the response.
 *
 * @returns Error number (from err.h). ERR_OK or ERR_INPROGRESS is returned on success.
 */
err_enum_t udp_single_operation(const char* hostname, uint16_t port, struct pbuf* p, uint32_t timeout, udp_sop_result_handler_fn result_handler, void* handler_data);

#ifdef __cplusplus
}
#endif
#endif // _KOB_NET_H_
