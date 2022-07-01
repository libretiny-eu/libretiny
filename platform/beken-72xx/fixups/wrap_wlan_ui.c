/* Copyright (c) Kuba Szczodrzyński 2022-07-01. */

#include <param_config.h>
#include <wlan_ui_pub.h>

// NOTE: this wrap is currently not used, as the related methods
// are in the same translation unit (wlan_ui.c), so wrapping doesn't work

extern void __real_bk_wlan_sta_init_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv);
extern sta_param_t *g_sta_param_ptr;

// enable/disable fast connect according to the config parameters
void __wrap_bk_wlan_sta_init_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv) {
	// let it do the job first
	__real_bk_wlan_sta_init_adv(inNetworkInitParaAdv);
	// correct the parameter
	bool fast_connect = false;
	if (inNetworkInitParaAdv->ap_info.channel) {
		// enable fast connect after finding first non-zero octet of BSSID
		for (uint8_t i = 0; i < 6; i++) {
			if (inNetworkInitParaAdv->ap_info.bssid[i] != 0x00) {
				fast_connect = true;
				break;
			}
		}
	}
	g_sta_param_ptr->fast_connect_set = fast_connect;
}
