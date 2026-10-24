/* Copyright (c) Kuba Szczodrzyński 2022-05-23. */

#if LT_HAS_LWIP2

#include "mDNS.h"
#include <WiFiUdp.h>
#include <map>
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

namespace {

constexpr uint32_t MDNS_QUERY_TIMEOUT_MS = 3000;
constexpr size_t MDNS_MAX_RESULTS		 = 20;
constexpr unsigned int DNS_MAX_JUMPS	 = 16;

constexpr uint16_t DNS_TYPE_A	 = 1;
constexpr uint16_t DNS_TYPE_PTR	 = 12;
constexpr uint16_t DNS_TYPE_TXT	 = 16;
constexpr uint16_t DNS_TYPE_AAAA = 28;
constexpr uint16_t DNS_TYPE_SRV	 = 33;

constexpr uint16_t DNS_CLASS_IN = 1;

constexpr uint16_t MDNS_PORT		  = 5353;
constexpr size_t MDNS_MAX_UDP_PAYLOAD = 9000 - 20 - 8;

constexpr uint16_t DNS_FLAG_RESPONSE = 0x8000;
constexpr uint16_t DNS_FLAG_OPCODE	 = 0x7800;
constexpr uint16_t DNS_FLAG_RCODE	 = 0x000F;
constexpr uint16_t DNS_CLASS_MASK	 = 0x7FFF;

struct QueryTxt {
	String key;
	String value;
};

struct QueryResult {
	String instance;
	String hostnameFqdn;
	String hostname;

	IPAddress ip;
	IPv6Address ipv6;

	uint16_t port = 0;

	std::vector<QueryTxt> txt;
};

struct DnsRecord {
	uint16_t type = 0;

	String name;
	String target;

	uint16_t port = 0;

	IPAddress ip;
	IPv6Address ipv6;

	std::vector<QueryTxt> txt;
};

static std::vector<QueryResult> sQueryResults;

static uint16_t dnsRead16(const uint8_t *p) {
	return ((uint16_t)p[0] << 8) | p[1];
}

static void dnsAppend16(std::vector<uint8_t> &packet, uint16_t value) {
	packet.push_back(value >> 8);
	packet.push_back(value & 0xFF);
}

static bool dnsNameEquals(const String &a, const String &b) {
	return strcasecmp(a.c_str(), b.c_str()) == 0;
}

static bool dnsAppendName(std::vector<uint8_t> &packet, const String &name) {
	int start  = 0;
	int length = name.length();

	while (start < length) {
		int dot = name.indexOf('.', start);

		if (dot < 0)
			dot = length;

		int labelLength = dot - start;

		if (labelLength <= 0 || labelLength > 63)
			return false;

		packet.push_back((uint8_t)labelLength);

		for (int i = start; i < dot; i++)
			packet.push_back((uint8_t)name[i]);

		start = dot + 1;
	}

	packet.push_back(0);

	return true;
}

/**
 * Reads a possibly compressed DNS name.
 *
 * 'offset' is advanced past the encoded name in the original stream,
 * even when the name contains compression pointers.
 */
static bool dnsReadName(const uint8_t *packet, size_t packetLength, size_t &offset, String &name) {
	size_t pos		  = offset;
	size_t nextOffset = offset;

	bool jumped		   = false;
	unsigned int jumps = 0;

	name = "";

	while (pos < packetLength) {
		uint8_t labelLength = packet[pos++];

		if (labelLength == 0) {
			offset = jumped ? nextOffset : pos;
			return true;
		}

		// DNS compression pointer: 11xxxxxx xxxxxxxx
		if ((labelLength & 0xC0) == 0xC0) {
			if (pos >= packetLength)
				return false;

			uint16_t pointer = ((uint16_t)(labelLength & 0x3F) << 8) | packet[pos++];

			if (pointer >= packetLength)
				return false;

			if (!jumped) {
				nextOffset = pos;
				jumped	   = true;
			}

			// protect against malformed pointer loops
			if (++jumps > DNS_MAX_JUMPS)
				return false;

			pos = pointer;
			continue;
		}

		if (labelLength & 0xC0)
			return false;

		if (labelLength > 63)
			return false;

		if (pos + labelLength > packetLength)
			return false;

		size_t expandedLength = name.length() + (name.length() ? 1 : 0) + labelLength;
		if (expandedLength > DNS_MAX_NAME_LENGTH)
			return false;

		if (name.length())
			name += '.';

		for (uint8_t i = 0; i < labelLength; i++)
			name += (char)packet[pos++];

		if (!jumped)
			offset = pos;
	}

	return false;
}

static bool dnsBuildQuery(const String &name, uint16_t type, uint16_t id, std::vector<uint8_t> &packet) {
	packet.clear();
	packet.resize(12, 0);

	// transaction ID
	packet[0] = id >> 8;
	packet[1] = id & 0xFF;

	// QDCOUNT = 1
	packet[4] = 0;
	packet[5] = 1;

	if (!dnsAppendName(packet, name))
		return false;

	dnsAppend16(packet, type);
	dnsAppend16(packet, DNS_CLASS_IN);

	return true;
}

static bool dnsParsePacket(
	const uint8_t *packet,
	size_t packetLength,
	uint16_t expectedId,
	std::vector<DnsRecord> &records
) {
	if (packetLength < 12)
		return false;

	uint16_t id = dnsRead16(packet);

	if (id != expectedId)
		return false;

	uint16_t flags = dnsRead16(packet + 2);

	// Must be a successful standard DNS response.
	if (!(flags & DNS_FLAG_RESPONSE))
		return false;

	if (flags & DNS_FLAG_OPCODE)
		return false;

	if (flags & DNS_FLAG_RCODE)
		return false;

	uint16_t questions	= dnsRead16(packet + 4);
	uint16_t answers	= dnsRead16(packet + 6);
	uint16_t authority	= dnsRead16(packet + 8);
	uint16_t additional = dnsRead16(packet + 10);

	size_t offset = 12;

	// Skip question section
	for (uint16_t i = 0; i < questions; i++) {
		String name;

		if (!dnsReadName(packet, packetLength, offset, name))
			return false;

		// QTYPE + QCLASS
		if (offset + 4 > packetLength)
			return false;

		offset += 4;
	}

	std::vector<DnsRecord> packetRecords;

	uint32_t recordCount = (uint32_t)answers + authority + additional;
	for (uint32_t i = 0; i < recordCount; i++) {
		DnsRecord record;

		if (!dnsReadName(packet, packetLength, offset, record.name))
			return false;

		// TYPE(2) CLASS(2) TTL(4) RDLENGTH(2)
		if (offset + 10 > packetLength)
			return false;

		record.type = dnsRead16(packet + offset);
		offset += 2;

		uint16_t recordClass = dnsRead16(packet + offset);
		offset += 2;

		// TTL
		offset += 4;

		uint16_t rdLength = dnsRead16(packet + offset);
		offset += 2;

		size_t rdata	= offset;
		size_t rdataEnd = rdata + rdLength;

		if (rdataEnd > packetLength)
			return false;

		if ((recordClass & DNS_CLASS_MASK) != DNS_CLASS_IN) {
			offset = rdataEnd;
			continue;
		}

		bool keep = false;

		switch (record.type) {
			case DNS_TYPE_PTR: {
				size_t nameOffset = rdata;

				if (dnsReadName(packet, packetLength, nameOffset, record.target)) {
					keep = true;
				}

				break;
			}

			case DNS_TYPE_SRV: {
				// priority(2) weight(2) port(2) target(...)
				if (rdLength < 6)
					break;

				record.port = dnsRead16(packet + rdata + 4);

				size_t nameOffset = rdata + 6;

				if (dnsReadName(packet, packetLength, nameOffset, record.target)) {
					keep = true;
				}

				break;
			}

			case DNS_TYPE_TXT: {
				size_t pos = rdata;

				while (pos < rdataEnd) {
					uint8_t txtLength = packet[pos++];

					if (pos + txtLength > rdataEnd)
						break;

					String item;

					for (uint8_t j = 0; j < txtLength; j++)
						item += (char)packet[pos + j];

					pos += txtLength;

					QueryTxt txt;

					int separator = item.indexOf('=');

					if (separator < 0) {
						txt.key	  = item;
						txt.value = "";
					} else {
						txt.key	  = item.substring(0, separator);
						txt.value = item.substring(separator + 1);
					}

					record.txt.push_back(txt);
				}

				keep = true;
				break;
			}

			case DNS_TYPE_A: {
				if (rdLength != 4)
					break;

				record.ip = IPAddress(packet[rdata + 0], packet[rdata + 1], packet[rdata + 2], packet[rdata + 3]);

				keep = true;
				break;
			}

			case DNS_TYPE_AAAA: {
				if (rdLength != 16)
					break;

				record.ipv6 = IPv6Address(packet + rdata);

				keep = true;
				break;
			}
		}

		if (keep)
			packetRecords.push_back(std::move(record));

		offset = rdataEnd;
	}

	// Set records only after a complete successful parse
	for (auto &record : packetRecords)
		records.push_back(std::move(record));

	return true;
}

static uint16_t dnsNextQueryId() {
	static uint16_t id = 0;

	id++;

	if (id == 0)
		id++;

	return id;
}

static bool dnsQuery(const String &name, uint16_t type, uint32_t timeout, std::vector<DnsRecord> &records) {
	const uint16_t id = dnsNextQueryId();

	std::vector<uint8_t> query;

	if (!dnsBuildQuery(name, type, id, query))
		return false;

	// Use an ephemeral source port so mDNS responders return legacy
	// unicast responses without conflicting with the local mDNS responder.
	WiFiUDP udp;

	if (!udp.beginPacket(IPAddress(224, 0, 0, 251), MDNS_PORT))
		return false;

	if (udp.write(query.data(), query.size()) != query.size()) {
		udp.stop();
		return false;
	}

	if (!udp.endPacket()) {
		udp.stop();
		return false;
	}

	uint32_t start = millis();

	while ((uint32_t)(millis() - start) < timeout) {
		int packetLength = udp.parsePacket();

		if (packetLength <= 0) {
			delay(5);
			continue;
		}

		if ((size_t)packetLength > MDNS_MAX_UDP_PAYLOAD) {
			LT_DM(MDNS, "Ignoring oversized mDNS packet: %u bytes", (unsigned)packetLength);
			udp.flush();
			continue;
		}

		// A valid legacy mDNS reply still originates from port 5353.
		if (udp.remotePort() != MDNS_PORT) {
			udp.flush();
			continue;
		}

		std::vector<uint8_t> response(packetLength);

		int readLength = udp.read(response.data(), response.size());

		if (readLength > 0) {
			dnsParsePacket(response.data(), readLength, id, records);
		}
	}

	udp.stop();

	return true;
}

static String dnsStripLocal(const String &hostname) {
	const char suffix[]		  = ".local";
	const size_t suffixLength = sizeof(suffix) - 1;

	if (hostname.length() > suffixLength) {
		const char *start = hostname.c_str() + hostname.length() - suffixLength;

		if (strcasecmp(start, suffix) == 0)
			return hostname.substring(0, hostname.length() - suffixLength);
	}

	return hostname;
}

static const QueryResult *getQueryResult(int idx) {
	if (idx < 0)
		return nullptr;

	if ((size_t)idx >= sQueryResults.size())
		return nullptr;

	return &sQueryResults[idx];
}

static QueryResult *findQueryResult(const String &instance) {
	for (auto &result : sQueryResults) {
		if (dnsNameEquals(result.instance, instance))
			return &result;
	}

	return nullptr;
}

static void buildQueryResults(const String &serviceName, const std::vector<DnsRecord> &records) {
	sQueryResults.clear();

	// PTR tells us which service instances exist.
	for (const auto &record : records) {
		if (record.type != DNS_TYPE_PTR)
			continue;

		if (!dnsNameEquals(record.name, serviceName))
			continue;

		if (findQueryResult(record.target))
			continue;

		if (sQueryResults.size() >= MDNS_MAX_RESULTS)
			break;

		QueryResult result;
		result.instance = record.target;

		sQueryResults.push_back(result);
	}

	// SRV and TXT belong to the service instance.
	for (auto &result : sQueryResults) {
		for (const auto &record : records) {
			if (!dnsNameEquals(record.name, result.instance))
				continue;

			if (record.type == DNS_TYPE_SRV) {
				result.hostnameFqdn = record.target;
				result.hostname		= dnsStripLocal(record.target);
				result.port			= record.port;
			} else if (record.type == DNS_TYPE_TXT) {
				result.txt = record.txt;
			}
		}
	}

	// A / AAAA belong to the host referenced by the SRV record.
	for (auto &result : sQueryResults) {
		if (!result.hostnameFqdn.length())
			continue;

		for (const auto &record : records) {
			if (!dnsNameEquals(record.name, result.hostnameFqdn))
				continue;

			if (record.type == DNS_TYPE_A)
				result.ip = record.ip;
			else if (record.type == DNS_TYPE_AAAA)
				result.ipv6 = record.ipv6;
		}
	}
}

} // namespace

struct CachedService {
	CachedService(const char *_name, const char *_service, mdns_sd_proto _proto, uint16_t _port)
		: name(strdup(_name)), service(strdup(_service)), proto(_proto), port(_port) {}

	CachedService(const CachedService &)			= delete;
	CachedService &operator=(const CachedService &) = delete;

	CachedService(CachedService &&other)
		: name(other.name), service(other.service), proto(other.proto), port(other.port),
		  records(std::move(other.records)) {
		other.name	  = nullptr;
		other.service = nullptr;
		other.records.clear();
	}

	~CachedService() {
		if (name) {
			free(name);
		}

		if (service) {
			free(service);
		}

		for (auto &str : records) {
			if (str) {
				free(str);
			}
		}
	}

	char *name;
	char *service;
	mdns_sd_proto proto;
	uint16_t port;
	std::vector<char *> records;
};

static std::vector<CachedService> sCachedServices;

static const char *hostName;
#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
NETIF_DECLARE_EXT_CALLBACK(netif_callback)
#endif

enum StateForNetIf {
	DISABLED = 0, // not yet enabled
	ENABLED,	  // enabled, but no services added
	CONFIGURED	  // enabled and services added
};

static std::map<uint8_t, StateForNetIf> sNetIfState; // netif->num  --> state

mDNS::mDNS() {}

mDNS::~mDNS() {
	cleanup();
}

void mDNS::cleanup() {
	sQueryResults.clear();

	sCachedServices.clear();

	free((void *)hostName);
	hostName = NULL;

	free((void *)instanceName);
	instanceName = NULL;
}

static void mdnsTxtCallback(struct mdns_service *service, void *userdata) {
	size_t index = (size_t)userdata;
	if (index >= sCachedServices.size())
		return;

	for (const auto &record : sCachedServices[index].records) {
		err_t err = mdns_resp_add_service_txtitem(service, record, strlen(record));
		if (err != ERR_OK) {
			LT_DM(MDNS, "Error %d while adding txt record: %s", err, record);
		}
	}
}

#if LWIP_VERSION_SIMPLE < 20200 // TTL removed in LwIP commit 62fb2fd749b (2.2.0 release)
static void mdnsStatusCallback(struct netif *netif, uint8_t result) {
	LT_DM(MDNS, "Status: netif %u, status %u", netif->num, result);
}
#else
static void mdnsStatusCallback(struct netif *netif, uint8_t result, int8_t slot) {
	LT_DM(MDNS, "Status: netif %u, status %u slot %d", netif->num, result, slot);
}
#endif

#ifdef LWIP_NETIF_EXT_STATUS_CALLBACK
static void addServices(struct netif *netif) {
	for (uint8_t i = 0; i < sCachedServices.size(); i++) {
		const auto &cachedService = sCachedServices[i];
		LT_DM(
			MDNS,
			"Add service: netif %u / %s / %s / %u / %u",
			netif->num,
			cachedService.name,
			cachedService.service,
			cachedService.proto,
			cachedService.port
		);
		s8_t slot = mdns_resp_add_service(
			netif,
			cachedService.name,
			cachedService.service,
			cachedService.proto,
			cachedService.port,
#if LWIP_VERSION_SIMPLE < 20200 // TTL removed in LwIP commit 62fb2fd749b (2.2.0 release)
			255,
#endif
			mdnsTxtCallback,
			reinterpret_cast<void *>(i) // index of newly added service
		);

		if (slot < 0) {
			LT_DM(MDNS, "mdns_resp_add_service returned error %d", slot);
		}
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
#if LWIP_VERSION_SIMPLE < 20200 // TTL removed in LwIP commit 62fb2fd749b (2.2.0 release)
		err_t ret = mdns_resp_add_netif(netif, hostName, 255);
#else
		err_t ret = mdns_resp_add_netif(netif, hostName);
#endif
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
static void mdns_netif_ext_status_callback(
	struct netif *netif,
	netif_nsc_reason_t reason,
	const netif_ext_callback_args_t *args
) {
	auto stateIter = sNetIfState.find(netif->num);
	if (stateIter == sNetIfState.end()) {
		stateIter = sNetIfState.insert(std::make_pair(netif->num, StateForNetIf::DISABLED)).first;
	}
	auto currentState = sNetIfState[netif->num];
	if (reason & LWIP_NSC_NETIF_REMOVED) {
		if (StateForNetIf::DISABLED != stateIter->second) {
			LT_DM(MDNS, "Netif removed, stopping mDNS on netif %u", netif->num);
			mdns_resp_remove_netif(netif);
			sNetIfState.erase(stateIter);
		}
	} else if ((reason & LWIP_NSC_STATUS_CHANGED) || (reason & LWIP_NSC_NETIF_ADDED)) {
		if (StateForNetIf::DISABLED == stateIter->second) {
			LT_DM(MDNS, "Starting mDNS on netif %u", netif->num);
			if (enableMDNS(netif)) {
				stateIter->second = StateForNetIf::ENABLED;
			}
		}

		if ((StateForNetIf::ENABLED == stateIter->second) && (sCachedServices.size() > 0)) {
			stateIter->second = StateForNetIf::CONFIGURED;
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
		sNetIfState[netif->num] = StateForNetIf::DISABLED;
		if (enableMDNS(netif)) {
			sNetIfState[netif->num] = StateForNetIf::ENABLED;
		}
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

	std::size_t serviceIndex = sCachedServices.size();
	// add the service to TXT record arrays
	sCachedServices.emplace_back(name, service, (mdns_sd_proto)proto, port);

	while (netif != NULL) {
		if (netif_is_up(netif)) {
			// register TXT callback;
			// pass service index as userdata parameter
			LT_DM(MDNS, "Add service: netif %u / %s / %s / %u / %u", netif->num, name, service, proto, port);
			s8_t slot = mdns_resp_add_service(
				netif,
				name,
				service,
				(mdns_sd_proto)proto,
				port,
#if LWIP_VERSION_SIMPLE < 20200 // TTL removed in LwIP commit 62fb2fd749b (2.2.0 release)
				255,
#endif
				mdnsTxtCallback,
				(void *)serviceIndex // index of newly added service
			);

			if (slot < 0) {
				LT_DM(MDNS, "mdns_resp_add_service returned error %d", slot);
			}

			added = true;
		}
		netif = netif->next;
	}

	return added;
}

bool mDNS::addServiceTxtImpl(const char *service, uint8_t proto, const char *item) {
	uint8_t i;
	for (i = 0; i < sCachedServices.size(); i++) {
		const auto &cachedService = sCachedServices[i];
		// find a matching service
		if (strcmp(cachedService.service, service) == 0 && cachedService.proto == proto) {
			break;
		}
	}
	if (i == sCachedServices.size())
		return false;

	sCachedServices[i].records.push_back(strdup(item));
	return true;
}

int mDNS::queryService(char *service, char *proto) {
	if (!service || !service[0] || !proto || !proto[0])
		return 0;

	sQueryResults.clear();

	String serviceName;

	if (service[0] == '_')
		serviceName = service;
	else {
		serviceName = "_";
		serviceName += service;
	}

	serviceName += ".";

	if (proto[0] == '_')
		serviceName += proto;
	else {
		serviceName += "_";
		serviceName += proto;
	}

	serviceName += ".local";

	LT_DM(MDNS, "Query service: %s", serviceName.c_str());

	std::vector<DnsRecord> records;

	if (!dnsQuery(serviceName, DNS_TYPE_PTR, MDNS_QUERY_TIMEOUT_MS, records)) {
		return 0;
	}

	LT_DM(MDNS, "Query service: received %u DNS records", (unsigned)records.size());

	buildQueryResults(serviceName, records);

	LT_DM(MDNS, "Query service: found %u services", (unsigned)sQueryResults.size());

	return (int)sQueryResults.size();
}

String mDNS::hostname(int idx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result)
		return "";

	return result->hostname;
}

IPAddress mDNS::IP(int idx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result)
		return IPAddress();

	return result->ip;
}

IPv6Address mDNS::IPv6(int idx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result)
		return IPv6Address();

	return result->ipv6;
}

uint16_t mDNS::port(int idx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result)
		return 0;

	return result->port;
}

int mDNS::numTxt(int idx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result)
		return 0;

	return (int)result->txt.size();
}

bool mDNS::hasTxt(int idx, const char *key) {
	const QueryResult *result = getQueryResult(idx);

	if (!result || !key)
		return false;

	for (const auto &txt : result->txt) {
		if (strcmp(txt.key.c_str(), key) == 0)
			return true;
	}

	return false;
}

String mDNS::txt(int idx, const char *key) {
	const QueryResult *result = getQueryResult(idx);

	if (!result || !key)
		return "";

	for (const auto &txt : result->txt) {
		if (strcmp(txt.key.c_str(), key) == 0)
			return txt.value;
	}

	return "";
}

String mDNS::txt(int idx, int txtIdx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result || txtIdx < 0)
		return "";

	if ((size_t)txtIdx >= result->txt.size())
		return "";

	return result->txt[txtIdx].value;
}

String mDNS::txtKey(int idx, int txtIdx) {
	const QueryResult *result = getQueryResult(idx);

	if (!result || txtIdx < 0)
		return "";

	if ((size_t)txtIdx >= result->txt.size())
		return "";

	return result->txt[txtIdx].key;
}

IPAddress mDNS::queryHost(char *host, uint32_t timeout) {
	if (!host || !host[0])
		return IPAddress();

	String hostname = host;

	if (hostname.length() < 6 || strcasecmp(hostname.c_str() + hostname.length() - 6, ".local") != 0) {
		hostname += ".local";
	}

	LT_DM(MDNS, "Query host: %s", hostname.c_str());

	std::vector<DnsRecord> records;

	if (!dnsQuery(hostname, DNS_TYPE_A, timeout, records)) {
		return IPAddress();
	}

	for (const auto &record : records) {
		if (record.type == DNS_TYPE_A && dnsNameEquals(record.name, hostname)) {
			return record.ip;
		}
	}

	return IPAddress();
}

MDNSResponder MDNS;

#endif

#endif
