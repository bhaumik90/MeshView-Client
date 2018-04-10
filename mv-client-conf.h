/**
 * \file
 *      MeshView Client Configuration Header File
 * \author
 *      Bhaumik Bhandari <bhaumik.bhandari@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef _MV_CLIENT_CONF_H_
#define _MV_CLIENT_CONF_H_
/*---------------------------------------------------------------------------*/
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    256
#endif

#ifndef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE     64
#endif

#ifndef MV_COMM_PERIOD
#define MV_COMM_PERIOD          600 * CLOCK_SECOND // 10 Minutes
#endif

#ifndef MV_SERVER
#define MV_SERVER(ipaddr)       uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x1)
#endif
/*---------------------------------------------------------------------------*/
#endif /* _MV_CLIENT_CONF_H_ */
