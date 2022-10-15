/* Copyright (c) Kuba Szczodrzyński 2022-05-23. */

#ifdef LT_HAS_LWIP2

#include "mDNS.h"
#include <vector>

extern "C" {
#include <errno.h>
#include <lwip/apps/mdns.h>
#include <lwip/igmp.h>
#include <lwip/init.h>
#include <lwip/netif.h>
}

static std::vector<char *> services;
static std::vector<uint8_t> protos;
static std::vector<std::vector<char *>> records;

mDNS::mDNS() {}

mDNS::~mDNS() {}

static void mdnsTxtCallback(struct mdns_service *service, void *userdata) {
	size_t index = (size_t)userdata;
	if (index >= records.size())
		return;

	for (const auto record : records[index]) {
		err_t err = mdns_resp_add_service_txtitem(service, record, strlen(record));
		if (err != ERR_OK)
			return;
	}
}

static void mdnsStatusCallback(struct netif *netif, uint8_t result) {
	LT_DM(MDNS, "Status: netif %u, status %u", netif->num, result);
}

bool mDNS::begin(const char *hostname) {
	setInstanceName(hostname);
	LT_DM(MDNS, "Starting (%s)", hostname);
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1
	mdns_resp_register_name_result_cb(mdnsStatusCallback);
#endif
	mdns_resp_init();
	uint8_t enabled = 0;

	struct netif *netif;
	for (netif = netif_list; netif != NULL; netif = netif->next) {
		if (!netif_is_up(netif))
			continue;

		LT_DM(MDNS, "Adding netif %u", netif->num);
		if ((netif->flags & NETIF_FLAG_IGMP) == 0) {
			LT_DM(MDNS, "Enabling IGMP");
			netif->flags |= NETIF_FLAG_IGMP;
			igmp_start(netif);
		}

		err_t ret = mdns_resp_add_netif(netif, hostname, 255);
		if (ret == ERR_OK)
			enabled++;
		else
			LT_DM(MDNS, "Cannot add netif %u; ret=%d, errno=%d", netif->num, ret, errno);
	}
	return enabled > 0;
}

void mDNS::end() {
	struct netif *netif = netif_list;
	while (netif != NULL) {
		if (netif_is_up(netif))
			mdns_resp_remove_netif(netif);
		netif = netif->next;
	}
}

bool mDNS::addServiceImpl(const char *name, const char *service, uint8_t proto, uint16_t port) {
	bool added			= false;
	struct netif *netif = netif_list;
	while (netif != NULL) {
		if (netif_is_up(netif)) {
			// register TXT callback;
			// pass service index as userdata parameter
			LT_DM(MDNS, "Add service: netif %u / %s / %s / %u / %u", netif->num, name, service, proto, port);
			mdns_resp_add_service(
				netif,
				name,
				service,
				(mdns_sd_proto)proto,
				port,
				255,
				mdnsTxtCallback,
				(void *)services.size() // index of newly added service
			);
			added = true;
		}
		netif = netif->next;
	}

	if (!added)
		return false;

	// add the service to TXT record arrays
	services.push_back(strdup(service));
	protos.push_back(proto);
	records.emplace_back();

	return true;
}

bool mDNS::addServiceTxtImpl(const char *service, uint8_t proto, const char *item) {
	int8_t index = -1;
	for (uint8_t i = 0; i < services.size(); i++) {
		// find a matching service
		if (strcmp(services[i], service) == 0 && protos[i] == proto) {
			index = i;
			break;
		}
	}
	if (index == -1)
		return false;

	records[index].push_back(strdup(item));
	return true;
}

MDNSResponder MDNS;

#endif
