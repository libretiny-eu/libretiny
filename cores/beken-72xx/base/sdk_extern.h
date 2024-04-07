/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <uart_pub.h>

// SDK
extern uint8_t system_mac[];
extern int uart_print_port;
uint32_t wdt_ctrl(uint32_t cmd, void *param);
void bk_send_byte(uint8_t uport, uint8_t data);
void uart_hw_set_change(uint8_t uport, bk_uart_config_t *uart_config);
int uart_rx_callback_set(int uport, uart_callback callback, void *param);
void sctrl_enter_rtos_deep_sleep(PS_DEEP_CTRL_PARAM *deep_param);
void ps_delay(volatile UINT16 times);
OSStatus bk_wlan_start_sta_adv_fix(network_InitTypeDef_adv_st *inNetworkInitParaAdv);

#ifdef __cplusplus
} // extern "C"
#endif
