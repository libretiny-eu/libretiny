/* Copyright (c) Bl00d-B0b 2026-08-10. */

#include <wifi_conf.h>
#include <wifi_util.h>

// Overrides the SDK's _WEAK wifi_set_mib() (wifi_conf.c): vendor defaults except
// 802.11v BTM / 802.11k disabled (client-steering controllers defer quick
// re-associations with assoc status 30) and power save starting disabled.
void wifi_set_mib(void) {
	wext_set_adaptivity(RTW_ADAPTIVITY_DISABLE);
	wext_auto_set_adaptivity(DISABLE);
	wext_set_trp_tis(RTW_TRP_TIS_DISABLE);
	wext_set_anti_interference(DISABLE);
	wext_set_roam_on_btm(0);
	wext_set_enable_80211k(0);
	wext_set_powersave_mode(0);
#ifdef CONFIG_SAE_SUPPORT
	wext_set_support_wpa3(ENABLE);
#endif
	wext_set_ant_div_gpio(0);
	wext_set_bw40_enable(0);
	wext_set_softap_gkey_rekey(DISABLE);
#ifdef CONFIG_80211N_HT
	wext_set_wifi_ampdu_tx(ENABLE);
#endif
}
