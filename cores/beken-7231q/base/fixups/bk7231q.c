/* Copyright (c) Kuba Szczodrzyński 2023-06-20. */

int ble_active			   = 0;
int ble_switch_mac_sleeped = 0;

int wifi_notice_ble_status() {
	return 0;
}

int wn_txl_hd_pending_is_allow() {
	return 1;
}

int if_ble_sleep() {
	return 1;
}

void ble_switch_rf_to_wifi() {}

int rwip_get_current_time() {
	return 0;
}

int rwip_get_next_target_time() {
	return 0;
}
