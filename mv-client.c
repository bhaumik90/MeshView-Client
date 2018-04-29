/**
 * \file
 *      MeshView Client
 * \author
 *      Bhaumik Bhandari <bhaumik.bhandari@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "mv-client.h"
#include "er-coap-engine.h"
#include "net/rpl/rpl.h"

#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif
/*---------------------------------------------------------------------------*/
PROCESS(mv_client, "MeshView Client");
/*---------------------------------------------------------------------------*/
static struct etimer mv_periodic_timer;
static uip_ipaddr_t mv_server_ipaddr;
static mv_eNwType_t nwType;
static mv_eNodeType_t nodeType;
extern resource_t res_led, res_config;
static bool isInited = false;
rpl_dag_t *dag = NULL;
/*---------------------------------------------------------------------------*/
static int
has_network_access(void)
{
#if UIP_CONF_IPV6_RPL
  if((dag = rpl_get_any_dag()) == NULL) {
    return 0;
  }
#endif /* UIP_CONF_IPV6_RPL */
  return 1;
}
/*---------------------------------------------------------------------------*/
static mv_eError_t
ipaddr_to_lladdr(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  mv_eError_t err = SUCCESS;
  verifyOrExit(ipaddr!=NULL && lladdr!=NULL, INVALID_ARG);

  memcpy(lladdr->addr, &ipaddr->u8[8], UIP_LLADDR_LEN);
  lladdr->addr[0] ^= 0x02;

  exit:
    return err;
}
/*---------------------------------------------------------------------------*/
static mv_eError_t 
node_info_create(mv_eNwType_t infoType, mv_eNodeType_t nodeType, uint8_t *buf, uint8_t *buf_len)
{
  mv_eError_t err = SUCCESS;
  verifyOrExit(buf!=NULL && buf_len!=NULL, INVALID_ARG);

  uint8_t len=0;

  // Adding network type
  buf[len] = MV_RPL; len++;

  // Adding NODE TYPE TLV
  buf[len] = NODE_TYPE_TLV; len++;
  buf[len] = 1; len++;
  buf[len] = nodeType; len++;

  // Adding PREFIX INFO TLV
  buf[len] = PREFIX_INFO_TLV; len++;
  buf[len] = 2; len++;
  memcpy(buf+len, dag->prefix_info.prefix.u8, 2);
  len += 2;

  // Adding NODE MAC TLV
  buf[len] = NODE_MAC_TLV; len++;
  buf[len] = UIP_LLADDR_LEN; len++;
  memcpy(buf+len, linkaddr_node_addr.u8, UIP_LLADDR_LEN);
  len += UIP_LLADDR_LEN;

  if(nodeType!=ROOT)
  {
    // Adding PARENT MAC TLV
    const linkaddr_t *parentlladdr = rpl_get_parent_lladdr(dag->preferred_parent);
    buf[len] = PARENT_MAC_TLV; len++;
    buf[len] = UIP_LLADDR_LEN; len++;
    memcpy(buf+len, parentlladdr->u8, UIP_LLADDR_LEN);
    len += UIP_LLADDR_LEN;
  }

  // Adding RANK TLV
  buf[len] = NODE_RANK_TLV; len++;
  buf[len] = sizeof(dag->rank); len++;
  buf[len] = (uint8_t) (dag->rank >> 8); len++;
  buf[len] = (uint8_t) dag->rank & 0xFF; len++;

  // Adding DODAG ID MAC TLV
  buf[len] = DODAG_ID_MAC_TLV; len++;
  buf[len] = UIP_LLADDR_LEN; len++;
  uip_lladdr_t tempLladdr;
  ipaddr_to_lladdr(&dag->dag_id, &tempLladdr);
  memcpy(buf+len, &tempLladdr, UIP_LLADDR_LEN);
  len += UIP_LLADDR_LEN;

  // Adding DODAG ID MAC TLV
  buf[len] = DAG_VERSION_TLV; len++;
  buf[len] = sizeof(dag->version); len++;
  memcpy(buf+len, &dag->version, sizeof(dag->version));
  len += sizeof(dag->version);

  radio_value_t val;

  // Adding Channel TLV
  buf[len] = CHANNEL_TLV; len++;
  buf[len] = 1; len++;
  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &val);
  memcpy(buf+len, &val, 1); len++;

  // Adding PAN ID TLV
  buf[len] = PAN_ID_TLV; len++;
  buf[len] = 2; len++;
  val = frame802154_get_pan_id();
  val = (val<<8 & 0xFF00) | (val>>8 & 0x00FF);
  memcpy(buf+len, &val, 2); len+=2;

  // Adding TX Power TLV
  buf[len] = TX_POWER_TLV; len++;
  buf[len] = 1; len++;
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val);
  memcpy(buf+len, &val, 1); len++;

  buf[len] = FW_VER_TLV; len++;
  buf[len] = 2; len++;
  val = (HWREGH(0x00002000)<<8 & 0xFF00) | (HWREGH(0x00002000)>>8 & 0x00FF);
  memcpy(buf+len, &val, 2); len+=2;
  
  *buf_len = len;

  exit:
    return err;
}
/*---------------------------------------------------------------------------*/
static void
client_chunk_handler(void *response)
{
  PRINTF("|%.*s\n", ((coap_packet_t *)response)->payload_len, (char *)((coap_packet_t *)response)->payload);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mv_client, ev, data)
{
  PROCESS_BEGIN();
  static coap_packet_t request[1];
  MV_SERVER(&mv_server_ipaddr);

  etimer_set(&mv_periodic_timer, CLOCK_SECOND);

  while(1) {
    PROCESS_YIELD();

    if(etimer_expired(&mv_periodic_timer))
    {
      if(!has_network_access()) 
      {
        etimer_reset(&mv_periodic_timer);
      }
      else
      {
        uint8_t buf[REST_MAX_CHUNK_SIZE]={0}, len=0;
        if(!node_info_create(nwType, nodeType, &buf[0], &len))
        {
          PRINTF("buf of len %d: ", len);
          for(int i=0; i<len; i++) PRINTF("%02x ", buf[i]);
          PRINTF("\n\r");
          coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
          coap_set_header_uri_path(request, "/mv");
          coap_set_header_content_format(request, APPLICATION_OCTET_STREAM);
          coap_set_payload(request, buf, len);
          COAP_BLOCKING_REQUEST(&mv_server_ipaddr, UIP_HTONS(COAP_DEFAULT_PORT), request, client_chunk_handler);
          etimer_reset_with_new_interval(&mv_periodic_timer, MV_COMM_PERIOD);
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
mv_client_init(mv_eNwType_t _nwType, mv_eNodeType_t _nodeType)
{
  if(isInited) return;
  isInited = true;
  nwType = _nwType;
  nodeType = _nodeType;
  process_start(&mv_client, NULL);
  coap_init_engine();
  rest_init_engine();
  rest_activate_resource(&res_led, "led");
  rest_activate_resource(&res_config, "config");
}
/*---------------------------------------------------------------------------*/
