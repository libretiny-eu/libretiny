/* Copyright (c) Kuba Szczodrzyński 2022-05-23. */

#if LT_HAS_LWIP2

#include "mDNS.h"
#include <vector>

extern "C" {
#include <errno.h>
#include <lwip/apps/mdns.h>
#include <lwip/igmp.h>
#include <lwip/init.h>
#include <lwip/netif.h>
}

#if LWIP_VERSION_SIMPLE < 20100 && defined(LWIP_NETIF_EXT_STATUS_CALLBACK)
#warning "LWIP_NETIF_EXT_STATUS_CALLBACK not available before lwIP 2.1.0"
#undef LWIP_NETIF_EXT_STATUS_CALLBACK
#endif

#if LWIP_MDNS_RESPONDER

static std::vector<char *> services_name;
static std::vector<char *> services;
static std::vector<uint8_t> protos;
static std::vector<uint16_t> ports;
static std::vector<std::vector<char *>> records;

static const char *hostName;
#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
NETIF_DECLARE_EXT_CALLBACK(netif_callback)
#endif

static inline void freeAllocatedStrings(const std::vector<char *> &strings) {
	for (auto &str : strings) {
		free(str);
	}
}

mDNS::mDNS() {}

mDNS::~mDNS() {
	cleanup();
}

void mDNS::cleanup() {
	freeAllocatedStrings(services_name);
	services_name.clear();
	freeAllocatedStrings(services);
	services.clear();
	for (auto &record : records) {
		freeAllocatedStrings(record);
	}
	records.clear();

	free((void *)hostName);
	hostName = NULL;

	free((void *)instanceName);
	instanceName = NULL;
}

static void mdnsTxtCallback(struct mdns_service *service, void *userdata) {
	size_t index = (size_t)userdata;
	if (index >= records.size())
		return;

	for (const auto record : records[index]) {
		err_t err = mdns_resp_add_service_txtitem(service, record, strlen(record));
		if (err != ERR_OK) {
			LT_DM(MDNS, "Error %d while adding txt record: %s", err, record);
		}
	}
}

static void mdnsStatusCallback(struct netif *netif, uint8_t result) {
	LT_DM(MDNS, "Status: netif %u, status %u", netif->num, result);
}

#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
static void addServices(struct netif *netif) {
	for (uint8_t i = 0; i < services.size(); i++) {
		LT_DM(
			MDNS,
			"Add service: netif %u / %s / %s / %u / %u",
			netif->num,
			services_name[i],
			services[i],
			protos[i],
			ports[i]
		);
		mdns_resp_add_service(
			netif,
			services_name[i],
			services[i],
			(mdns_sd_proto)protos[i],
			ports[i],
			255,
			mdnsTxtCallback,
			reinterpret_cast<void *>(i) // index of newly added service
		);
	}
}
#endif

static bool enableMDNS(struct netif *netif) {
	if (netif_is_up(netif)) {
		LT_DM(MDNS, "Starting mDNS on netif %u", netif->num);
		if ((netif->flags & NETIF_FLAG_IGMP) == 0) {
			netif->flags |= NETIF_FLAG_IGMP;
			igmp_start(netif);
			LT_DM(MDNS, "Added IGMP to netif %u", netif->num);
		}
		err_t ret = mdns_resp_add_netif(netif, hostName, 255);
		if (ret == ERR_OK) {
			LT_DM(MDNS, "mDNS started on netif %u, announcing it to network", netif->num);
#if LWIP_VERSION_SIMPLE >= 20100
			mdns_resp_announce(netif);
#else
#warning "lwIP version older than 2.1.0, mdns_resp_announce() unavailable"
#endif
			return true;
		} else {
			LT_DM(MDNS, "Cannot start mDNS on netif %u; ret=%d, errno=%d", netif->num, ret, errno);
		}
	}
	return false;
}

#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
static void
mdns_netif_ext_status_callback(struct netif *netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t *args) {
	if (reason & LWIP_NSC_NETIF_REMOVED) {
		LT_DM(MDNS, "Netif removed, stopping mDNS on netif %u", netif->num);
		mdns_resp_remove_netif(netif);
	} else if (reason & LWIP_NSC_STATUS_CHANGED) {
		LT_DM(MDNS, "Netif changed, starting mDNS on netif %u", netif->num);
		if (enableMDNS(netif) && services.size() > 0) {
			LT_DM(MDNS, "Adding services to netif %u", netif->num);
			addServices(netif);
		}
	}
}
#endif

bool mDNS::begin(const char *hostname) {
	hostName = strdup(hostname);
	setInstanceName(hostname);
#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
	netif_add_ext_callback(&netif_callback, mdns_netif_ext_status_callback);
#endif
	LT_DM(MDNS, "Starting (%s)", hostname);
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1
	mdns_resp_register_name_result_cb(mdnsStatusCallback);
#endif
	mdns_resp_init();
	struct netif *netif;
	for (netif = netif_list; netif != NULL; netif = netif->next) {
		enableMDNS(netif);
	}
	return true;
}

void mDNS::end() {
#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
	netif_remove_ext_callback(&netif_callback);
#endif

	struct netif *netif = netif_list;
	while (netif != NULL) {
		if (netif_is_up(netif))
			mdns_resp_remove_netif(netif);
		netif = netif->next;
	}

	cleanup();
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
	services_name.push_back(strdup(name));
	services.push_back(strdup(service));
	protos.push_back(proto);
	ports.push_back(port);
	records.emplace_back();

	return true;
}

bool mDNS::addServiceTxtImpl(const char *service, uint8_t proto, const char *item) {
	uint8_t i;
	for (i = 0; i < services.size(); i++) {
		// find a matching service
		if (strcmp(services[i], service) == 0 && protos[i] == proto) {
			break;
		}
	}
	if (i == services.size())
		return false;

	records[i].push_back(strdup(item));
	return true;
}

MDNSResponder MDNS;

#endif

#endif
