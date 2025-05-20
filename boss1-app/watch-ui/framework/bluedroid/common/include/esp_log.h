#pragma once

#include <syslog.h>

#include "bt_common.h"
#include "bt_user_config.h"

typedef enum {
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;

#define ESP_LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { syslog(LOG_ERR, "[%s]" format, tag, ##__VA_ARGS__); }

#define ESP_LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)    { syslog(LOG_WARNING, "[%s]" format, tag, ##__VA_ARGS__); }

#define ESP_LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)    { syslog(LOG_INFO, "[%s]" format, tag,  ##__VA_ARGS__); }

#define ESP_LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { syslog(LOG_DEBUG, "[%s]" format, tag, ##__VA_ARGS__); }

#define ESP_LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { syslog(LOG_DEBUG, "[%s]" format, tag, ##__VA_ARGS__); }
