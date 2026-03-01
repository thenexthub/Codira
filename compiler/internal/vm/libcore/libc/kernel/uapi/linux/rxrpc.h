/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_RXRPC_H
#define _UAPI_LINUX_RXRPC_H
#include <linux/types.h>
#include <linux/in.h>
#include <linux/in6.h>
struct sockaddr_rxrpc {
  __kernel_sa_family_t srx_family;
  __u16 srx_service;
  __u16 transport_type;
  __u16 transport_len;
  union {
    __kernel_sa_family_t family;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } transport;
};
#define RXRPC_SECURITY_KEY 1
#define RXRPC_SECURITY_KEYRING 2
#define RXRPC_EXCLUSIVE_CONNECTION 3
#define RXRPC_MIN_SECURITY_LEVEL 4
#define RXRPC_UPGRADEABLE_SERVICE 5
#define RXRPC_SUPPORTED_CMSG 6
#define RXRPC_MANAGE_RESPONSE 7
enum rxrpc_cmsg_type {
  RXRPC_USER_CALL_ID = 1,
  RXRPC_ABORT = 2,
  RXRPC_ACK = 3,
  RXRPC_NET_ERROR = 5,
  RXRPC_BUSY = 6,
  RXRPC_LOCAL_ERROR = 7,
  RXRPC_NEW_CALL = 8,
  RXRPC_EXCLUSIVE_CALL = 10,
  RXRPC_UPGRADE_SERVICE = 11,
  RXRPC_TX_LENGTH = 12,
  RXRPC_SET_CALL_TIMEOUT = 13,
  RXRPC_CHARGE_ACCEPT = 14,
  RXRPC_OOB_ID = 15,
  RXRPC_CHALLENGED = 16,
  RXRPC_RESPOND = 17,
  RXRPC_RESPONDED = 18,
  RXRPC_RESP_RXGK_APPDATA = 19,
  RXRPC__SUPPORTED
};
#define RXRPC_SECURITY_PLAIN 0
#define RXRPC_SECURITY_AUTH 1
#define RXRPC_SECURITY_ENCRYPT 2
#define RXRPC_SECURITY_NONE 0
#define RXRPC_SECURITY_RXKAD 2
#define RXRPC_SECURITY_RXGK 4
#define RXRPC_SECURITY_RXK5 5
#define RXRPC_SECURITY_YFS_RXGK 6
#define RX_CALL_DEAD - 1
#define RX_INVALID_OPERATION - 2
#define RX_CALL_TIMEOUT - 3
#define RX_EOF - 4
#define RX_PROTOCOL_ERROR - 5
#define RX_USER_ABORT - 6
#define RX_ADDRINUSE - 7
#define RX_DEBUGI_BADTYPE - 8
#define RXGEN_CC_MARSHAL - 450
#define RXGEN_CC_UNMARSHAL - 451
#define RXGEN_SS_MARSHAL - 452
#define RXGEN_SS_UNMARSHAL - 453
#define RXGEN_DECODE - 454
#define RXGEN_OPCODE - 455
#define RXGEN_SS_XDRFREE - 456
#define RXGEN_CC_XDRFREE - 457
#define RXKADINCONSISTENCY 19270400
#define RXKADPACKETSHORT 19270401
#define RXKADLEVELFAIL 19270402
#define RXKADTICKETLEN 19270403
#define RXKADOUTOFSEQUENCE 19270404
#define RXKADNOAUTH 19270405
#define RXKADBADKEY 19270406
#define RXKADBADTICKET 19270407
#define RXKADUNKNOWNKEY 19270408
#define RXKADEXPIRED 19270409
#define RXKADSEALEDINCON 19270410
#define RXKADDATALEN 19270411
#define RXKADILLEGALLEVEL 19270412
#define RXGK_INCONSISTENCY 1233242880
#define RXGK_PACKETSHORT 1233242881
#define RXGK_BADCHALLENGE 1233242882
#define RXGK_SEALEDINCON 1233242883
#define RXGK_NOTAUTH 1233242884
#define RXGK_EXPIRED 1233242885
#define RXGK_BADLEVEL 1233242886
#define RXGK_BADKEYNO 1233242887
#define RXGK_NOTRXGK 1233242888
#define RXGK_UNSUPPORTED 1233242889
#define RXGK_GSSERROR 1233242890
struct rxrpc_challenge {
  __u16 service_id;
  __u8 security_index;
  __u8 pad;
};
struct rxgk_challenge {
  struct rxrpc_challenge base;
  __u32 enctype;
};
#endif
