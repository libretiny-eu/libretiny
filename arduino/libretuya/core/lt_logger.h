/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

#include "LibreTuyaConfig.h"
#include <stdint.h>

#if LT_LOGGER_CALLER
#define LT_LOG(level, caller, line, ...) lt_log(level, caller, line, __VA_ARGS__)
#define LT_LOGM(level, module, caller, line, ...)                                                                      \
	do {                                                                                                               \
		if (LT_DEBUG_##module) {                                                                                       \
			lt_log(level, caller, line, #module ": " __VA_ARGS__);                                                     \
		}                                                                                                              \
	} while (0)
void lt_log(const uint8_t level, const char *caller, const unsigned short line, const char *format, ...)
	__attribute__((format(printf, 4, 5)));
#else
#define LT_LOG(level, caller, line, ...) lt_log(level, __VA_ARGS__)
#define LT_LOGM(level, module, caller, line, ...)                                                                      \
	do {                                                                                                               \
		if (LT_DEBUG_##module) {                                                                                       \
			lt_log(level, #module ": " __VA_ARGS__);                                                                   \
		}                                                                                                              \
	} while (0)
void lt_log(const uint8_t level, const char *format, ...) __attribute__((format(printf, 2, 3)));
#endif

/**
 * @brief Change log output port.
 *
 * @param port UART port index - can be 0, 1 or 2
 */
void lt_log_set_port(uint8_t port);

/**
 * @brief Disable LT logger. Enable it back using lt_log_set_port(LT_UART_DEFAULT_LOGGER).
 */
void lt_log_disable();

#if LT_LEVEL_TRACE >= LT_LOGLEVEL
#define LT_T(...)		   LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_V(...)		   LT_LOG(LT_LEVEL_TRACE, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_TM(module, ...) LT_LOGM(LT_LEVEL_TRACE, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_VM(module, ...) LT_LOGM(LT_LEVEL_TRACE, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_T(...)
#define LT_V(...)
#define LT_TM(...)
#define LT_VM(...)
#endif

#if LT_LEVEL_DEBUG >= LT_LOGLEVEL
#define LT_D(...)		   LT_LOG(LT_LEVEL_DEBUG, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_DM(module, ...) LT_LOGM(LT_LEVEL_DEBUG, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_D(...)
#define LT_DM(...)
#endif

#if LT_LEVEL_INFO >= LT_LOGLEVEL
#define LT_I(...)		   LT_LOG(LT_LEVEL_INFO, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_IM(module, ...) LT_LOGM(LT_LEVEL_INFO, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_I(...)
#define LT_IM(...)
#endif

#if LT_LEVEL_WARN >= LT_LOGLEVEL
#define LT_W(...)		   LT_LOG(LT_LEVEL_WARN, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_WM(module, ...) LT_LOGM(LT_LEVEL_WARN, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_W(...)
#define LT_WM(...)
#endif

#if LT_LEVEL_ERROR >= LT_LOGLEVEL
#define LT_E(...)		   LT_LOG(LT_LEVEL_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_EM(module, ...) LT_LOGM(LT_LEVEL_ERROR, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_E(...)
#define LT_EM(...)
#endif

#if LT_LEVEL_FATAL >= LT_LOGLEVEL
#define LT_F(...)		   LT_LOG(LT_LEVEL_FATAL, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LT_FM(module, ...) LT_LOGM(LT_LEVEL_FATAL, module, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define LT_F(...)
#define LT_FM(...)
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
