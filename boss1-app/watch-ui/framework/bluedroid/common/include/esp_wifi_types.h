/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef __ESP_WIFI_TYPES_H__
#define __ESP_WIFI_TYPES_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WIFI_MODE_NULL = 0,  /**< null mode */
    WIFI_MODE_STA,       /**< WiFi station mode */
    WIFI_MODE_AP,        /**< WiFi soft-AP mode */
    WIFI_MODE_APSTA,     /**< WiFi station + soft-AP mode */
    WIFI_MODE_NAN,       /**< WiFi NAN mode */
    WIFI_MODE_MAX
} wifi_mode_t;

/* Strength of authmodes */
/* OPEN < WEP < WPA_PSK < OWE < WPA2_PSK = WPA_WPA2_PSK < WAPI_PSK < WPA3_PSK = WPA2_WPA3_PSK */
typedef enum {
    WIFI_AUTH_OPEN = 0,         /**< authenticate mode : open */
    WIFI_AUTH_WEP,              /**< authenticate mode : WEP */
    WIFI_AUTH_WPA_PSK,          /**< authenticate mode : WPA_PSK */
    WIFI_AUTH_WPA2_PSK,         /**< authenticate mode : WPA2_PSK */
    WIFI_AUTH_WPA_WPA2_PSK,     /**< authenticate mode : WPA_WPA2_PSK */
    WIFI_AUTH_ENTERPRISE,       /**< authenticate mode : WiFi EAP security */
    WIFI_AUTH_WPA2_ENTERPRISE = WIFI_AUTH_ENTERPRISE,  /**< authenticate mode : WiFi EAP security */
    WIFI_AUTH_WPA3_PSK,         /**< authenticate mode : WPA3_PSK */
    WIFI_AUTH_WPA2_WPA3_PSK,    /**< authenticate mode : WPA2_WPA3_PSK */
    WIFI_AUTH_WAPI_PSK,         /**< authenticate mode : WAPI_PSK */
    WIFI_AUTH_OWE,              /**< authenticate mode : OWE */
    WIFI_AUTH_WPA3_ENT_192,     /**< authenticate mode : WPA3_ENT_SUITE_B_192_BIT */
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

typedef enum {
    WIFI_REASON_UNSPECIFIED                        = 1,
    WIFI_REASON_AUTH_EXPIRE                        = 2,
    WIFI_REASON_AUTH_LEAVE                         = 3,
    WIFI_REASON_ASSOC_EXPIRE                       = 4,
    WIFI_REASON_ASSOC_TOOMANY                      = 5,
    WIFI_REASON_NOT_AUTHED                         = 6,
    WIFI_REASON_NOT_ASSOCED                        = 7,
    WIFI_REASON_ASSOC_LEAVE                        = 8,
    WIFI_REASON_ASSOC_NOT_AUTHED                   = 9,
    WIFI_REASON_DISASSOC_PWRCAP_BAD                = 10,
    WIFI_REASON_DISASSOC_SUPCHAN_BAD               = 11,
    WIFI_REASON_BSS_TRANSITION_DISASSOC            = 12,
    WIFI_REASON_IE_INVALID                         = 13,
    WIFI_REASON_MIC_FAILURE                        = 14,
    WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT             = 15,
    WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT           = 16,
    WIFI_REASON_IE_IN_4WAY_DIFFERS                 = 17,
    WIFI_REASON_GROUP_CIPHER_INVALID               = 18,
    WIFI_REASON_PAIRWISE_CIPHER_INVALID            = 19,
    WIFI_REASON_AKMP_INVALID                       = 20,
    WIFI_REASON_UNSUPP_RSN_IE_VERSION              = 21,
    WIFI_REASON_INVALID_RSN_IE_CAP                 = 22,
    WIFI_REASON_802_1X_AUTH_FAILED                 = 23,
    WIFI_REASON_CIPHER_SUITE_REJECTED              = 24,
    WIFI_REASON_TDLS_PEER_UNREACHABLE              = 25,
    WIFI_REASON_TDLS_UNSPECIFIED                   = 26,
    WIFI_REASON_SSP_REQUESTED_DISASSOC             = 27,
    WIFI_REASON_NO_SSP_ROAMING_AGREEMENT           = 28,
    WIFI_REASON_BAD_CIPHER_OR_AKM                  = 29,
    WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION       = 30,
    WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS        = 31,
    WIFI_REASON_UNSPECIFIED_QOS                    = 32,
    WIFI_REASON_NOT_ENOUGH_BANDWIDTH               = 33,
    WIFI_REASON_MISSING_ACKS                       = 34,
    WIFI_REASON_EXCEEDED_TXOP                      = 35,
    WIFI_REASON_STA_LEAVING                        = 36,
    WIFI_REASON_END_BA                             = 37,
    WIFI_REASON_UNKNOWN_BA                         = 38,
    WIFI_REASON_TIMEOUT                            = 39,
    WIFI_REASON_PEER_INITIATED                     = 46,
    WIFI_REASON_AP_INITIATED                       = 47,
    WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT      = 48,
    WIFI_REASON_INVALID_PMKID                      = 49,
    WIFI_REASON_INVALID_MDE                        = 50,
    WIFI_REASON_INVALID_FTE                        = 51,
    WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED = 67,
    WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED        = 68,

    WIFI_REASON_BEACON_TIMEOUT                     = 200,
    WIFI_REASON_NO_AP_FOUND                        = 201,
    WIFI_REASON_AUTH_FAIL                          = 202,
    WIFI_REASON_ASSOC_FAIL                         = 203,
    WIFI_REASON_HANDSHAKE_TIMEOUT                  = 204,
    WIFI_REASON_CONNECTION_FAIL                    = 205,
    WIFI_REASON_AP_TSF_RESET                       = 206,
    WIFI_REASON_ROAMING                            = 207,
    WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG       = 208,
    WIFI_REASON_SA_QUERY_TIMEOUT                   = 209,
} wifi_err_reason_t;

#ifdef __cplusplus
}
#endif

#endif /* __ESP_WIFI_TYPES_H__ */
