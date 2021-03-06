/**
 * \file
 *      MeshView Client Configuration Resource
 * \author
 *      Bhaumik Bhandari <bhaumik.bhandari@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "mv-client.h"
#include "rest-engine.h"
#include "er-coap.h"
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
static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
/*---------------------------------------------------------------------------*/
RESOURCE(res_config,
         "title=\"Configuration Resources\" PUT;rt=\"actuator\"",
         NULL,
         NULL,
         res_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
static void
res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

  coap_packet_t *const req = (coap_packet_t *)request;
  
	for(uint16_t i=0; i<req->payload_len;)
  {
    switch(req->payload[i])
    {
      case TX_POWER_TLV:
        i++; i++;
        NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, (int8_t)req->payload[i]);
        i++;
        break;
    }
  }
  REST.set_response_status(response, REST.status.CHANGED);
}
/*---------------------------------------------------------------------------*/
