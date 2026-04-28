/* Copyright (c) Kuba Szczodrzyński 2026-04-03. */

#include "include.h"

#if CFG_LOW_VOLTAGE_PS
// TODO: this should be implemented in lwIP (calls etharp_reply()), but I was too lazy to do that,
// and it probably won't hurt to just not call it at all :)
void net_send_gratuitous_arp() {}
#endif
