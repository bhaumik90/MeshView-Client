/**
 * \file
 *      MeshView Client Header File
 * \author
 *      Bhaumik Bhandari <bhaumik.bhandari@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef _MV_CLIENT_H_
#define _MV_CLIENT_H_
/*---------------------------------------------------------------------------*/
#include "mv-client-conf.h"
/*---------------------------------------------------------------------------*/
#define verifyOrExit(cond, e)  if(!(cond)) {err=e; goto exit;}
/*---------------------------------------------------------------------------*/
typedef enum {
  NODE_TYPE_TLV=0,
  PREFIX_INFO_TLV,
  NODE_MAC_TLV,
  PARENT_MAC_TLV,
  NODE_RANK_TLV,
  DODAG_ID_MAC_TLV,
  DAG_VERSION_TLV,
  CHANNEL_TLV,
  PAN_ID_TLV,
  TX_POWER_TLV,
  FW_VER_TLV
} mv_eTlvType_t;

typedef enum {
  SUCCESS = 0,
  INVALID_ARG,
} mv_eError_t;

typedef enum {
  MV_RPL = 1
} mv_eNwType_t;

typedef enum {
  ROOT = 0,
  ROUTER,
  END_NODE
} mv_eNodeType_t;
/*---------------------------------------------------------------------------*/
void mv_client_init(mv_eNwType_t, mv_eNodeType_t);
/*---------------------------------------------------------------------------*/
#endif /* _MV_CLIENT_H_ */
