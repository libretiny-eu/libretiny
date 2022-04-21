#ifndef __NETBIOS_H__
#define __NETBIOS_H__

/** default port number for "NetBIOS Name service */
#define NETBIOS_PORT     137

/** size of a NetBIOS name */
#define NETBIOS_NAME_LEN 16

#ifndef NET_IF_NUM
#define NET_IF_NUM 2
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern char netbios_name[NET_IF_NUM][NETBIOS_NAME_LEN + 1]; // default netifs/interfacenum: 0 - SoftAP, 1 - Station, 2 - Ethernet

// struct udp_pcb * netbios_pcb(void);
void netbios_init(void);
bool netbios_set_name(unsigned char interfacenum, char * name); // default netifs/interfacenum: 0 - SoftAP, 1 - Station, 2 - Ethernet
bool netbios_off(void);

#ifdef __cplusplus
}
#endif

#endif /* __NETBIOS_H__ */
