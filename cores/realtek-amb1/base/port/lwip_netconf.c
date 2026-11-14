/* Ameba1 WLAN glue for LibreTiny's lwIP2 (not the SDK's lwIP1.4). */
#include <FreeRTOS.h>
#include <ethernetif.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/netifapi.h>
#include <lwip/prot/dhcp.h>
#include <lwip_netconf.h>
#include <semphr.h>
#include <task.h>

struct netif xnetif[NET_IF_NUM];
int lwip_init_done;
unsigned char dhcp_mode_sta = 1;

/* Boot-crash instrumentation, see base/fixups/boot_trace.c (temporary). */
extern void lt_amb1_trace(const char *tag);
extern void lt_amb1_trace2(const char *tag, uint32_t value);
extern void lt_amb1_trace_tcm(const char *tag);
extern void lt_amb1_app_log_test(const char *tag);

/* A failed tcpip/netif initialization is terminal until reboot: tcpip_init
 * cannot safely be repeated, nor may partially registered netifs be re-added. */
enum { INIT_NEW, INIT_BUSY, INIT_READY, INIT_FAILED };

static int init_state;

static int initState(void) {
	taskENTER_CRITICAL();
	int state = init_state;
	taskEXIT_CRITICAL();
	return state;
}

bool lt_amb1_lwip_ready(void) {
	return initState() == INIT_READY;
}

static void tcpipReady(void *arg) {
	/* Runs on the freshly created tcpip thread: proves the thread started and
	 * the semaphore is about to be given. Printed via the ROM console path, so
	 * it appears even when the app's own log backend is not working. */
	lt_amb1_trace("tcpip callback fired (tcpip thread alive)");
	xSemaphoreGive((SemaphoreHandle_t)arg);
}

void LwIP_Init(void) {
	lt_amb1_trace("net stack -> LwIP_Init");
	lt_amb1_trace_tcm("tcm at LwIP_Init");
	lt_amb1_app_log_test("app-log OK at LwIP_Init");
	taskENTER_CRITICAL();
	int previous = init_state;
	if (previous == INIT_NEW)
		init_state = INIT_BUSY;
	taskEXIT_CRITICAL();
	if (previous != INIT_NEW) {
		lt_amb1_trace2("LwIP_Init re-entry, state", (uint32_t)previous);
		while (initState() == INIT_BUSY)
			vTaskDelay(1);
		return;
	}
	SemaphoreHandle_t ready = xSemaphoreCreateBinary();
	if (!ready) {
		lt_amb1_trace("LwIP ready semaphore FAILED");
		goto failed;
	}
	lt_amb1_trace("LwIP tcpip_init enter");
	tcpip_init(tcpipReady, ready);
	lt_amb1_trace("LwIP tcpip_init returned, waiting tcpip-ready");
	xSemaphoreTake(ready, portMAX_DELAY);
	lt_amb1_trace("LwIP tcpip-ready (took tcpip callback semaphore)");
	vSemaphoreDelete(ready);
	ip4_addr_t zero = {0};
	for (unsigned i = 0; i < NET_IF_NUM; i++) {
		xnetif[i].name[0] = 'r';
		xnetif[i].name[1] = '0' + i;
		lt_amb1_trace2("LwIP netif_add idx", (uint32_t)i);
		err_t result = netifapi_netif_add(&xnetif[i], &zero, &zero, &zero, NULL, ethernetif_init, tcpip_input);
		if (result != ERR_OK) {
			lt_amb1_trace2("LwIP netif_add FAILED idx", (uint32_t)i);
			goto failed;
		}
	}
	lt_amb1_trace("LwIP set_default");
	if (netifapi_netif_set_default(&xnetif[0]) != ERR_OK) {
		lt_amb1_trace("LwIP set_default FAILED");
		goto failed;
	}
	taskENTER_CRITICAL();
	lwip_init_done = 1;
	init_state	   = INIT_READY;
	taskEXIT_CRITICAL();
	lt_amb1_trace("LwIP_Init READY");
	return;
failed:
	taskENTER_CRITICAL();
	init_state = INIT_FAILED;
	taskEXIT_CRITICAL();
	lt_amb1_trace("LwIP_Init FAILED (state=INIT_FAILED)");
}

/* netifapi invokes this snapshot on the tcpip thread; never retain a DHCP
 * client pointer while another thread can stop or release that client. */
static err_t dhcpStatus(struct netif *netif) {
	if (dhcp_supplied_address(netif))
		return ERR_OK;
	struct dhcp *client = netif_dhcp_data(netif);
	if (!client || client->state == DHCP_STATE_OFF || !netif_is_link_up(netif))
		return ERR_IF;
	return ERR_INPROGRESS;
}

uint8_t LwIP_DHCP(uint8_t idx, uint8_t state) {
	static uint8_t traced;
	if (!traced && state == DHCP_START) {
		traced = 1;
		lt_amb1_trace2("LwIP_DHCP first start, idx", (uint32_t)idx);
	}
	if (!lt_amb1_lwip_ready() || idx >= NET_IF_NUM) {
		lt_amb1_trace("LwIP_DHCP called before ready");
		return DHCP_STOP;
	}
	struct netif *netif = &xnetif[idx];
	if (state == DHCP_STOP || state == DHCP_RELEASE_IP) {
		netifapi_dhcp_release_and_stop(netif);
		return DHCP_STOP;
	}
	if (state != DHCP_START && state != DHCP_WAIT_ADDRESS)
		return state;
	if (state == DHCP_START) {
		netifapi_netif_set_up(netif);
		if (netifapi_dhcp_start(netif) != ERR_OK)
			return DHCP_TIMEOUT;
	}
	TickType_t start = xTaskGetTickCount();
	while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(30000)) {
		err_t status = netifapi_netif_common(netif, NULL, dhcpStatus);
		if (status == ERR_OK)
			return DHCP_ADDRESS_ASSIGNED;
		if (status != ERR_INPROGRESS)
			return DHCP_STOP;
		vTaskDelay(pdMS_TO_TICKS(100));
	}
	netifapi_dhcp_stop(netif);
	return DHCP_TIMEOUT;
}

void LwIP_ReleaseIP(uint8_t idx) {
	LwIP_DHCP(idx, DHCP_RELEASE_IP);
}

unsigned char *LwIP_GetMAC(struct netif *netif) {
	return netif->hwaddr;
}

unsigned char *LwIP_GetIP(struct netif *netif) {
	return (unsigned char *)netif_ip4_addr(netif);
}

unsigned char *LwIP_GetGW(struct netif *netif) {
	return (unsigned char *)netif_ip4_gw(netif);
}

uint8_t *LwIP_GetMASK(struct netif *netif) {
	return (uint8_t *)netif_ip4_netmask(netif);
}

uint8_t *LwIP_GetBC(struct netif *netif) {
	static ip4_addr_t broadcast;
	broadcast.addr = netif_ip4_addr(netif)->addr | ~netif_ip4_netmask(netif)->addr;
	return (uint8_t *)&broadcast;
}

void LwIP_GetDNS(struct ip_addr *dns) {
	*dns = *dns_getserver(0);
}

void LwIP_SetDNS(struct ip_addr *dns) {
	dns_setserver(0, dns);
}

void LwIP_UseStaticIP(struct netif *netif) {
	netifapi_dhcp_stop(netif);
}

uint32_t LWIP_Get_Dynamic_Sleep_Interval(void) {
	return 0;
}
