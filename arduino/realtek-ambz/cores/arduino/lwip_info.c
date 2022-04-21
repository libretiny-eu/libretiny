#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip_netconf.h"
#include "lwip/udp.h"
#include "lwip/tcpip.h"
#include "lwip/err.h"
//#include "lwip/mem.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"

/* Get one byte from the 4-byte address */
#define ip4_addr1(ipaddr) (((u8_t*)(ipaddr))[0])
#define ip4_addr2(ipaddr) (((u8_t*)(ipaddr))[1])
#define ip4_addr3(ipaddr) (((u8_t*)(ipaddr))[2])
#define ip4_addr4(ipaddr) (((u8_t*)(ipaddr))[3])
/* These are cast to u16_t, with the intent that they are often arguments
 * to printf using the U16_F format from cc.h. */
#define ip4_addr1_16(ipaddr) ((u16_t)ip4_addr1(ipaddr))
#define ip4_addr2_16(ipaddr) ((u16_t)ip4_addr2(ipaddr))
#define ip4_addr3_16(ipaddr) ((u16_t)ip4_addr3(ipaddr))
#define ip4_addr4_16(ipaddr) ((u16_t)ip4_addr4(ipaddr))

#define IP2STR(ipaddr) ip4_addr1_16(ipaddr), \
    ip4_addr2_16(ipaddr), \
    ip4_addr3_16(ipaddr), \
    ip4_addr4_16(ipaddr)

#define IPSTR "%d.%d.%d.%d"

extern const char * const tcp_state_str[];
/*
static const char * const tcp_state_str[] = {
  "CLOSED",
  "LISTEN",
  "SYN_SENT",
  "SYN_RCVD",
  "ESTABLISHED",
  "FIN_WAIT_1",
  "FIN_WAIT_2",
  "CLOSE_WAIT",
  "CLOSING",
  "LAST_ACK",
  "TIME_WAIT"
};
*/
/******************************************************************************
 * FunctionName : debug
 * Parameters   :
 * Returns      :
*******************************************************************************/
void print_udp_pcb(void)
{
  struct udp_pcb *pcb;
  bool prt_none = true;
  rtl_printf("UDP pcbs:\n");
  for(pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
	  rtl_printf("flg:%02x\t" IPSTR ":%d\t" IPSTR ":%d\trecv:%p\n", pcb->flags, IP2STR(&pcb->local_ip), pcb->local_port, IP2STR(&pcb->remote_ip), pcb->remote_port, pcb->recv );
	  prt_none = false;
  }
  if(prt_none) rtl_printf("none\n");
}
/******************************************************************************
 * FunctionName : debug
 * Parameters   :
 * Returns      :
*******************************************************************************/
void print_tcp_pcb(void)
{
  struct tcp_pcb *pcb;
  rtl_printf("Active PCB states:\n");
  bool prt_none = true;
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
     rtl_printf("Port %d|%d\tflg:%02x\ttmr:%p\t%s\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, tcp_state_str[pcb->state]);
     prt_none = false;
  }
  if(prt_none) rtl_printf("none\n");
  rtl_printf("Listen PCB states:\n");
  prt_none = true;
  for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next) {
    rtl_printf("Port %d|%d\tflg:%02x\ttmr:%p\t%s\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, tcp_state_str[pcb->state]);
    prt_none = false;
  }
  if(prt_none) rtl_printf("none\n");
  rtl_printf("TIME-WAIT PCB states:\n");
  prt_none = true;
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
    rtl_printf("Port %d|%d\tflg:%02x\ttmr:%p\t%s\n", pcb->local_port, pcb->remote_port, pcb->flags, pcb->tmr, tcp_state_str[pcb->state]);
    prt_none = false;
  }
  if(prt_none) rtl_printf("none\n");
}
/******************************************************************************
 * FunctionName : debug
 * Parameters   :
 * Returns      :
*******************************************************************************/
#if 0
//------------------------------------------------------------------------------
void chow_tcp_connection_info(void)
{
        rtl_printf("TCP Server connections:\n");
        TCP_SERV_CFG * p;
        TCP_SERV_CONN * ts_conn;
        bool prt_none = true;
        for(p  = phcfg; p != NULL; p = p->next) {
        	for(ts_conn  = p->conn_links; ts_conn != NULL; ts_conn = ts_conn->next) {
        		rtl_printf("%d "IPSTR ":%d %s\n", p->port, ts_conn->remote_ip.b[0], ts_conn->remote_ip.b[1], ts_conn->remote_ip.b[2], ts_conn->remote_ip.b[3], ts_conn->remote_port, tspsrv_srvconn_state_msg(ts_conn->state) );
        	    prt_none = false;
        	}
        }
        if(prt_none) rtl_printf("none\n");
}
#endif

#ifdef __cplusplus
}
#endif
