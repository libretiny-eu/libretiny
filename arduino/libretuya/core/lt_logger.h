/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

#include "LibreTuyaConfig.h"
#include <stdint.h>

#if LT_LOGGER_CALLER
#define LT_LOG(level, caller, line, ...) lt_log(level, caller, line, __VA_ARGS__)
void lt_log(const uint8_t level, const char *caller, const unsigned short line, const char *format, ...);
#else
#define LT_LOG(level, caller, line, ...) lt_log(level, __VA_ARGS__)
void lt_log(const uint8_t level, const char *format, ...);
#endif

/**
 * @brief Change log output port.
 *
 * @param port UART port index - can be 0, 1 or 2
 */
void lt_log_set_port(uint8_t port);

#if LT_LEVEL_TRACE >= LT_LOGLEVEL
#define LT_T(...) LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_V(...) LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_T(...)
#define LT_V(...)
#endif

#if LT_LEVEL_DEBUG >= LT_LOGLEVEL
#define LT_D(...) LT_LOG(LT_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_D(...)
#endif

#if LT_LEVEL_INFO >= LT_LOGLEVEL
#define LT_I(...) LT_LOG(LT_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_I(...)
#endif

#if LT_LEVEL_WARN >= LT_LOGLEVEL
#define LT_W(...) LT_LOG(LT_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_W(...)
#endif

#if LT_LEVEL_ERROR >= LT_LOGLEVEL
#define LT_E(...) LT_LOG(LT_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_E(...)
#endif

#if LT_LEVEL_FATAL >= LT_LOGLEVEL
#define LT_F(...) LT_LOG(LT_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_F(...)
#endif

#if LT_LOG_HEAP
#define LT_HEAP_I() LT_I("Free heap: %u", LT_HEAP_FUNC());
#else
#define LT_HEAP_I()
#endif

// ESP32 compat
#define log_printf(...)		LT_I(__VA_ARGS__)
#define log_v(...)			LT_V(__VA_ARGS__)
#define log_d(...)			LT_D(__VA_ARGS__)
#define log_i(...)			LT_I(__VA_ARGS__)
#define log_w(...)			LT_W(__VA_ARGS__)
#define log_e(...)			LT_E(__VA_ARGS__)
#define log_n(...)			LT_E(__VA_ARGS__)
#define isr_log_v(...)		LT_V(__VA_ARGS__)
#define isr_log_d(...)		LT_D(__VA_ARGS__)
#define isr_log_i(...)		LT_I(__VA_ARGS__)
#define isr_log_w(...)		LT_W(__VA_ARGS__)
#define isr_log_e(...)		LT_E(__VA_ARGS__)
#define isr_log_n(...)		LT_E(__VA_ARGS__)
#define ESP_LOGV(...)		LT_V(__VA_ARGS__)
#define ESP_LOGD(...)		LT_D(__VA_ARGS__)
#define ESP_LOGI(...)		LT_I(__VA_ARGS__)
#define ESP_LOGW(...)		LT_W(__VA_ARGS__)
#define ESP_LOGE(...)		LT_E(__VA_ARGS__)
#define ESP_EARLY_LOGV(...) LT_V(__VA_ARGS__)
#define ESP_EARLY_LOGD(...) LT_D(__VA_ARGS__)
#define ESP_EARLY_LOGI(...) LT_I(__VA_ARGS__)
#define ESP_EARLY_LOGW(...) LT_W(__VA_ARGS__)
#define ESP_EARLY_LOGE(...) LT_E(__VA_ARGS__)
#define ets_printf(...)		LT_I(__VA_ARGS__)
#define ETS_PRINTF(...)		LT_I(__VA_ARGS__)

#define LT_T_MOD(module, ...)                                                                                          \
	do {                                                                                                               \
		if (module) {                                                                                                  \
			LT_T(__VA_ARGS__);                                                                                         \
		}                                                                                                              \
	} while (0)

#define LT_D_MOD(module, ...)                                                                                          \
	do {                                                                                                               \
		if (module) {                                                                                                  \
			LT_D(__VA_ARGS__);                                                                                         \
		}                                                                                                              \
	} while (0)

#define LT_RET(ret)                                                                                                    \
	LT_E("ret=%d", ret);                                                                                               \
	return ret;

#define LT_RET_NZ(ret)                                                                                                 \
	if (ret) {                                                                                                         \
		LT_E("ret=%d", ret);                                                                                           \
		return ret;                                                                                                    \
	}
#define LT_RET_LZ(ret)                                                                                                 \
	if (ret < 0) {                                                                                                     \
		LT_E("ret=%d", ret);                                                                                           \
		return ret;                                                                                                    \
	}
#define LT_RET_LEZ(ret)                                                                                                \
	if (ret <= 0) {                                                                                                    \
		LT_E("ret=%d", ret);                                                                                           \
		return ret;                                                                                                    \
	}

#define LT_ERRNO_NZ(ret)                                                                                               \
	if (ret) {                                                                                                         \
		LT_E("errno=%d, ret=%d", errno, ret);                                                                          \
		return ret;                                                                                                    \
	}
#define LT_ERRNO_LZ(ret)                                                                                               \
	if (ret < 0) {                                                                                                     \
		LT_E("errno=%d, ret=%d", errno, ret);                                                                          \
		return ret;                                                                                                    \
	}
#define LT_ERRNO_LEZ(ret)                                                                                              \
	if (ret <= 0) {                                                                                                    \
		LT_E("errno=%d, ret=%d", errno, ret);                                                                          \
		return ret;                                                                                                    \
	}

#if LT_LOG_ERRNO
#define LT_ERRNO()                                                                                                     \
	if (errno) {                                                                                                       \
		LT_E("errno=%d", errno);                                                                                       \
		errno = 0;                                                                                                     \
	}
#else
#define LT_ERRNO()
#endif

// WiFi.cpp
#define LT_T_WG(...) LT_T_MOD(LT_DEBUG_WIFI, __VA_ARGS__)
#define LT_V_WG(...) LT_T_MOD(LT_DEBUG_WIFI, __VA_ARGS__)
#define LT_D_WG(...) LT_D_MOD(LT_DEBUG_WIFI, __VA_ARGS__)

// WiFiClient.cpp
#define LT_T_WC(...) LT_T_MOD(LT_DEBUG_WIFI_CLIENT, __VA_ARGS__)
#define LT_V_WC(...) LT_T_MOD(LT_DEBUG_WIFI_CLIENT, __VA_ARGS__)
#define LT_D_WC(...) LT_D_MOD(LT_DEBUG_WIFI_CLIENT, __VA_ARGS__)

// WiFiServer.cpp
#define LT_T_WS(...) LT_T_MOD(LT_DEBUG_WIFI_SERVER, __VA_ARGS__)
#define LT_V_WS(...) LT_T_MOD(LT_DEBUG_WIFI_SERVER, __VA_ARGS__)
#define LT_D_WS(...) LT_D_MOD(LT_DEBUG_WIFI_SERVER, __VA_ARGS__)

// WiFiSTA.cpp
#define LT_T_WSTA(...) LT_T_MOD(LT_DEBUG_WIFI_STA, __VA_ARGS__)
#define LT_V_WSTA(...) LT_T_MOD(LT_DEBUG_WIFI_STA, __VA_ARGS__)
#define LT_D_WSTA(...) LT_D_MOD(LT_DEBUG_WIFI_STA, __VA_ARGS__)

// WiFiAP.cpp
#define LT_T_WAP(...) LT_T_MOD(LT_DEBUG_WIFI_AP, __VA_ARGS__)
#define LT_V_WAP(...) LT_T_MOD(LT_DEBUG_WIFI_AP, __VA_ARGS__)
#define LT_D_WAP(...) LT_D_MOD(LT_DEBUG_WIFI_AP, __VA_ARGS__)

// WiFiClientSecure.cpp & implementations
#define LT_T_SSL(...) LT_T_MOD(LT_DEBUG_SSL, __VA_ARGS__)
#define LT_V_SSL(...) LT_T_MOD(LT_DEBUG_SSL, __VA_ARGS__)
#define LT_D_SSL(...) LT_D_MOD(LT_DEBUG_SSL, __VA_ARGS__)
