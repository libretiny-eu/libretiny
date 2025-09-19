/* Copyright (c) Kuba Szczodrzyński 2022-07-07. */

// Fix for compiling on BK7231N with CFG_USE_TEMPERATURE_DETECT=0
// Method is used by libuart_debug_bk7231n.a / bkreg_run_command_implement()

void temp_detect_change_configuration(unsigned long intval, unsigned long thre, unsigned long dist) {}
