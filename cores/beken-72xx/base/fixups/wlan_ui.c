#include "include.h"
#include "wlan_ui_pub.h"
#include "rw_pub.h"
#include "param_config.h"
#include "wpa_psk_cache.h"

extern void bk_wlan_sta_init_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv);

extern void bk_wlan_sta_adv_param_2_sta(network_InitTypeDef_adv_st *sta_adv,network_InitTypeDef_st* sta);

OSStatus bk_wlan_start_sta_adv_fix(network_InitTypeDef_adv_st *inNetworkInitParaAdv)
{
    if (bk_wlan_is_monitor_mode()) {
        return 0;
    }

#if !CFG_WPA_CTRL_IFACE
#if CFG_ROLE_LAUNCH
    rl_status_set_private_state(RL_PRIV_STATUS_STA_ADV_RDY);
#endif
#if CFG_ROLE_LAUNCH
    rl_status_set_st_state(RL_ST_STATUS_RESTART_ST);
#endif

    bk_wlan_stop(BK_STATION);
#if CFG_ROLE_LAUNCH
    if (rl_pre_sta_set_status(RL_STATUS_STA_INITING))
        return -1;

    rl_status_reset_st_state(RL_ST_STATUS_RESTART_HOLD | RL_ST_STATUS_RESTART_ST);
    rl_status_set_private_state(RL_PRIV_STATUS_STA_ADV);
    rl_status_reset_private_state(RL_PRIV_STATUS_STA_ADV_RDY);
    LAUNCH_REQ lreq;

    lreq.req_type = LAUNCH_REQ_STA;
    bk_wlan_sta_adv_param_2_sta(inNetworkInitParaAdv, &lreq.descr);

    rl_sta_adv_register_cache_station(&lreq);
#endif

    bk_wlan_sta_init_adv(inNetworkInitParaAdv);

    /*
     * Key buffer is not guaranteed to be null terminated: there is a separate
     * `key_len` field that stores the length. We have to copy the data to a
     * temporary stack buffer. Fortunately, `wpa_psk_request()` does not keep
     * the reference, as it's not needed: the end result is PSK that is either
     * derived from the `ssid` + `key` or is already available. `key` is just a
     * transient argument.
     */
    {
        char key_buffer[PMK_LEN + 1];

        /* Make copy with a guaranteed null terminator. */
        memcpy(key_buffer, g_sta_param_ptr->key, g_sta_param_ptr->key_len);
        key_buffer[g_sta_param_ptr->key_len] = '\0';

        /*
         * let wpa_psk_cal thread to caculate psk.
         * XXX: If you know psk value, fill last two parameters of `wpa_psk_request()'.
         */
        wpa_psk_request(g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length,
                        key_buffer, NULL, 0);
    }

    supplicant_main_entry(inNetworkInitParaAdv->ap_info.ssid);

    net_wlan_add_netif(&g_sta_param_ptr->own_mac);


#else /* CFG_WPA_CTRL_IFACE */
    wlan_sta_config_t config;

#if (CFG_SOC_NAME == SOC_BK7231N)
    if (get_ate_mode_state()) {
        // cunliang20210407 set blk_standby_cfg with blk_txen_cfg like txevm, qunshan confirmed
        rwnx_cal_en_extra_txpa();
    }
#endif
    bk_wlan_sta_init_adv(inNetworkInitParaAdv);

    /*
     * Key buffer is not guaranteed to be null terminated: there is a separate
     * `key_len` field that stores the length. We have to copy the data to a
     * temporary stack buffer. Fortunately, `wpa_psk_request()` does not keep
     * the reference, as it's not needed: the end result is PSK that is either
     * derived from the `ssid` + `key` or is already available. `key` is just a
     * transient argument.
     */
    {
        char key_buffer[PMK_LEN + 1];

        /* Make copy with a guaranteed null terminator. */
        memcpy(key_buffer, g_sta_param_ptr->key, g_sta_param_ptr->key_len);
        key_buffer[g_sta_param_ptr->key_len] = '\0';

        /*
         * let wpa_psk_cal thread to caculate psk.
         * XXX: If you know psk value, fill last two parameters of `wpa_psk_request()'.
         */
        wpa_psk_request(g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length,
                        key_buffer, NULL, 0);
    }

    /* disconnect previous connected network */
    wlan_sta_disconnect();

    /* start wpa_supplicant */
    wlan_sta_enable();

    /* set network parameters: ssid, passphase */
    wlan_sta_set((uint8_t*)inNetworkInitParaAdv->ap_info.ssid, strlen(inNetworkInitParaAdv->ap_info.ssid),
                 (uint8_t*)inNetworkInitParaAdv->key);

    /* set fast connect bssid */
    memset(&config, 0, sizeof(config));
    /*
     * This API has no way to communicate whether BSSID was supplied or not.
     * So we will use an assumption that no valid endpoint will ever use the
     * `00:00:00:00:00:00`. It's basically a MAC address and no valid device
     * should ever use the value consisting of all zeroes.
     *
     * We have just zeroed out the `config` struct so we can use those zeroes
     * for comparison with our input argument.
     */
    if (memcmp(config.u.bssid, inNetworkInitParaAdv->ap_info.bssid, ETH_ALEN)) {
        memcpy(config.u.bssid, inNetworkInitParaAdv->ap_info.bssid, ETH_ALEN);
        config.field = WLAN_STA_FIELD_BSSID;
        wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config);
    }

    /* set fast connect freq */
    memset(&config, 0, sizeof(config));
    config.u.channel = inNetworkInitParaAdv->ap_info.channel;
    config.field = WLAN_STA_FIELD_FREQ;
    wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config);

    /* connect to AP */
    wlan_sta_connect(g_sta_param_ptr->fast_connect_set ? g_sta_param_ptr->fast_connect.chann : 0);

#endif
    ip_address_set(BK_STATION, inNetworkInitParaAdv->dhcp_mode,
                   inNetworkInitParaAdv->local_ip_addr,
                   inNetworkInitParaAdv->net_mask,
                   inNetworkInitParaAdv->gateway_ip_addr,
                   inNetworkInitParaAdv->dns_server_ip_addr);

    return 0;
}

/*
 * Make the SDK's 802.11n weak-RSSI fallback threshold configurable.
 *
 * The rwnx MAC blob (scanu.o: scanu_change_ht_supported) compares the RSSI
 * of the join-scan result against a threshold, and if the frame is weaker,
 * clears the STA's HT capability (me_env.ht_supported) before associating.
 * The device then associates as 802.11g-only. HT is only restored at the
 * start of the next scan (scanu_recover_ht_supported), so whether a given
 * association ends up 11n or 11g is effectively racy across reboots.
 *
 * The threshold defaults to -50 dBm unless the weak hook
 * rwnx_get_noht_rssi_thresold() (declared in
 * beken378/ip/lmac/src/rwnx/rwnx_config.h) is defined. Define it here so
 * the threshold can be set at build time via -DLT_BK_NOHT_RSSI_THRESHOLD.
 *
 * The blob truncates the return value to signed 8-bit, so valid values are
 * -128..-1. This lives in wlan_ui.c (not its own file) so that it is always
 * extracted from the fixups archive: the blob's reference to the hook is
 * weak and would not pull a standalone object out of the library.
 */
#ifndef LT_BK_NOHT_RSSI_THRESHOLD
#define LT_BK_NOHT_RSSI_THRESHOLD -50
#endif

__attribute__((weak)) int rwnx_get_noht_rssi_thresold(void) {
    return LT_BK_NOHT_RSSI_THRESHOLD;
}
