/* Copyright (c) Kuba Szczodrzyński 2022-08-26. */

#include "mDNS.h"

static char *ensureUnderscore(const char *value) {
	uint8_t len	 = strlen(value) + 1;
	char *result = (char *)malloc(len);
	result[0]	 = '_';
	strcpy(result + 1, value + (value[0] == '_'));
	return result;
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
	free(_service);
	return result;
}

bool mDNS::addServiceTxt(char *service, char *proto, char *key, char *value) {
	char *_service = ensureUnderscore(service);
	uint8_t _proto = strncmp(proto + (proto[0] == '_'), "tcp", 3) == 0 ? MDNS_TCP : MDNS_UDP;

	uint8_t txt_len = strlen(key) + strlen(value) + 1;
	char *txt		= (char *)malloc(txt_len + 1);
	sprintf(txt, "%s=%s", key, value);

	bool result = addServiceTxtImpl(_service, _proto, txt);
	free(_service);
	free(txt);
	return result;
}
