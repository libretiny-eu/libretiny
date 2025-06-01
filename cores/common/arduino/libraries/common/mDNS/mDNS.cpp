/* Copyright (c) Kuba Szczodrzyński 2022-08-26. */

#include "mDNS.h"

static char *ensureUnderscore(char *value) {
	if (value[0] == '_') {
		return value;
	}
	size_t len	 = strlen(value) + 1 + 1; // 1 for underscore, 1 for null-terminator
	char *result = (char *)malloc(len);
	result[0]	 = '_';
	strcpy(result + 1, value);
	return result;
}

static inline void freeIfCopied(const char *original, char *duplicate) {
	if ((duplicate) && (original != duplicate)) {
		free(duplicate);
	}
}

void mDNS::setInstanceName(const char *name) {
	if (instanceName)
		free(instanceName);
	instanceName = strdup(name);
}

bool mDNS::addService(char *service, char *proto, uint16_t port) {
	char *_service = ensureUnderscore(service);
	uint8_t _proto = strncmp(proto + (proto[0] == '_'), "tcp", 3) == 0 ? MDNS_TCP : MDNS_UDP;

	bool result = addServiceImpl(instanceName ? instanceName : "LT mDNS", _service, _proto, port);
	freeIfCopied(service, _service);
	return result;
}

bool mDNS::addServiceTxt(char *service, char *proto, char *key, char *value) {
	char *_service = ensureUnderscore(service);
	uint8_t _proto = strncmp(proto + (proto[0] == '_'), "tcp", 3) == 0 ? MDNS_TCP : MDNS_UDP;

	uint8_t txt_len = strlen(key) + strlen(value) + 1;
	char *txt		= (char *)malloc(txt_len + 1);
	sprintf(txt, "%s=%s", key, value);

	bool result = addServiceTxtImpl(_service, _proto, txt);
	freeIfCopied(service, _service);
	free(txt);
	return result;
}
