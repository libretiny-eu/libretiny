/* Copyright (c) Kuba Szczodrzyński 2022-05-23. */

#ifdef LT_HAS_LWIP2

#include "mDNS.h"

extern "C" {
#include <lwip/apps/mdns.h>
#include <lwip/netif.h>
}

static u8_t mdns_netif_client_id = 0; // TODO fix this

struct mdns_domain {
	/* Encoded domain name */
	u8_t name[256];
	/* Total length of domain name, including zero */
	u16_t length;
	/* Set if compression of this domain is not allowed */
	u8_t skip_compression;
};

/** Description of a service */
struct mdns_service {
	/** TXT record to answer with */
	struct mdns_domain txtdata;
	/** Name of service, like 'myweb' */
	char name[MDNS_LABEL_MAXLEN + 1];
	/** Type of service, like '_http' */
	char service[MDNS_LABEL_MAXLEN + 1];
	/** Callback function and userdata
	 * to update txtdata buffer */
	service_get_txt_fn_t txt_fn;
	void *txt_userdata;
	/** TTL in seconds of SRV/TXT replies */
	u32_t dns_ttl;
	/** Protocol, TCP or UDP */
	u16_t proto;
	/** Port of the service */
	u16_t port;
};

/** Description of a host/netif */
struct mdns_host {
	/** Hostname */
	char name[MDNS_LABEL_MAXLEN + 1];
	/** Pointer to services */
	struct mdns_service *services[MDNS_MAX_SERVICES];
	/** TTL in seconds of A/AAAA/PTR replies */
	u32_t dns_ttl;
};

static String mdnsInstanceName = "default_instance";

mDNS::mDNS() {}

mDNS::~mDNS() {}

bool mDNS::begin(const char *hostname) {
	mdns_resp_init();
	struct netif *netif = netif_list;
	uint8_t enabled		= 0;
	while (netif != NULL) {
		netif->flags |= NETIF_FLAG_IGMP;
		// TODO: detect mdns_netif_client_id by checking netif_get_client_data()
		// and finding the requested hostname in struct mdns_host
		if (netif_is_up(netif) && mdns_resp_add_netif(netif, hostname, 255) == ERR_OK) {
			enabled++;
		}
		netif = netif->next;
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

void mDNS::setInstanceName(String name) {
	mdnsInstanceName = name;
}

bool mDNS::addService(char *service, char *proto, uint16_t port) {
	char _service[strlen(service) + 2];
	char _proto[strlen(proto) + 2];
	_service[0] = '_';
	_proto[0]	= '_';
	// prepend names with _
	strcpy(_service + 1, service + (service[0] == '_'));
	strcpy(_proto + 1, proto + (proto[0] == '_'));

	mdns_sd_proto protocol = DNSSD_PROTO_UDP;
	if (strncmp(_proto + 1, "tcp", 3) == 0)
		protocol = DNSSD_PROTO_TCP;

	struct netif *netif = netif_list;
	while (netif != NULL) {
		if (netif_is_up(netif)) {
			mdns_resp_add_service(netif, mdnsInstanceName.c_str(), _service, protocol, port, 255, NULL, NULL);
		}
		netif = netif->next;
	}
}

bool mDNS::addServiceTxt(char *name, char *proto, char *key, char *value) {
	char _name[strlen(name) + 2];
	char _proto[strlen(proto) + 2];
	_name[0]  = '_';
	_proto[0] = '_';
	// prepend names with _
	strcpy(_name + 1, name + (name[0] == '_'));
	strcpy(_proto + 1, proto + (proto[0] == '_'));

	mdns_sd_proto protocol = DNSSD_PROTO_UDP;
	if (strncmp(_proto + 1, "tcp", 3) == 0)
		protocol = DNSSD_PROTO_TCP;

	struct netif *netif = netif_list;
	struct mdns_host *mdns;
	struct mdns_service *service;

	uint8_t txt_len = strlen(key) + strlen(value) + 1;
	char *txt		= (char *)malloc(txt_len + 1);
	sprintf(txt, "%s=%s", key, value);

	while (netif != NULL) {
		if (netif_is_up(netif)) {
			mdns = (struct mdns_host *)netif_get_client_data(netif, mdns_netif_client_id);

			for (uint8_t i = 0; i < MDNS_MAX_SERVICES; i++) {
				service = mdns->services[i];
				if (service == NULL)
					continue;
				if (strcmp(service->service, _name) || service->proto != protocol)
					continue;
				if (mdns_resp_add_service_txtitem(service, txt, txt_len) != ERR_OK) {
					free(txt);
					return false;
				}
			}
		}
		netif = netif->next;
	}
	free(txt);
	return true;
}

MDNSResponder MDNS;

#endif
