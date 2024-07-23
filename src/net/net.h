/**
 * Network functionaly
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
 */
#ifndef _NET_H_
#define _NET_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include "system_defs.h"
#include "pico/stdlib.h"
#include <string.h>

#ifdef BOARD_IS_PICOW
#include "pico/cyw43_arch.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#else
/** Definitions for error constants. */
typedef enum {
/** No error, everything OK. */
  ERR_OK         = 0,
} err_enum_t;
#endif // BOARD_IS_PICOW

#define NET_SSID_MAX_LEN 32
#define NET_PASSWORD_MAX_LEN 128
#define NET_URL_MAX_LEN 2048

typedef struct pbuf pbuf_t;

/**
 * @brief Get the 'host' portion of a 'host:port' identifier.
 *
 * @param buf Buffer to copy the 'host' portion into.
 * @param maxlen Maximum length to copy.
 * @param host_and_port The 'host:port' to process. The ':port' portion can be missing.
 * @return The length of the host name (could be larger than maxlen)
 */
int host_from_hostport(char* buf, uint32_t maxlen, const char* host_and_port);

/**
 * @brief Get the 'port' from a 'host:port' identifier.
 *
 * @param host_and_port The 'host:port' to process.
 * @param port_default The port number to return if the identifier didn't include a ':port' part.
 * @return uint16_t The port.
 */
uint16_t port_from_hostport(const char* host_and_port, uint16_t port_default);

/**
 * @brief Make a NTP network call and use the result to update the board's RTC.
 * @ingroup wire
 *
 * @param tz_offset Hours offset from UTC. For a timezone like UTC−09:30, use a value of -9.5
 * @returns ERR_OK (0) on success
 */
err_enum_t network_update_rtc(float tz_offset);

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

#ifdef __cplusplus
}
#endif
#endif // _NET_H_
