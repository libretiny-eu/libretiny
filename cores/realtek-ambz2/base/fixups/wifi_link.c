#include <libretiny.h>
#include <lwip/netifapi.h>

// Keep SDK link-up calls on the lwIP thread, like station disconnects.
void lt_ambz2_netif_set_link_up(struct netif *netif) {
	err_t err = netifapi_netif_set_link_up(netif);
	if (err != ERR_OK)
		LT_EM(WIFI, "Failed to set WiFi link up: %d", err);
}
