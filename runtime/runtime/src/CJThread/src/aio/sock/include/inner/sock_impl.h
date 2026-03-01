/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef MRT_SOCK_IMPL_H
#define MRT_SOCK_IMPL_H

#include <stdbool.h>
#include "sock.h"
#include "log.h"
#ifdef MRT_WINDOWS
#include <winsock2.h>
#include <mswsock.h>
#include <afunix.h>
#include <windows.h>
#else
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* Maximum number of connections that can be listened on by default. The actual value is the smaller.
 * value between the value of this parameter and the value of this parameter in
 * "/proc/sys/net/core/somaxconn". */
#define SO_MAX_CONN 1024
#define SOMAXCONN_HINT(b) (-(b))
#define RECV_MAX_CYCLES 15
#define PROCESSOR_FREE_RATIO 2

/* Supported network type. The value ranges from 0 to 63 (6 bits). */
enum SockNetType {
    NET_TYPE_TCP = 0,
    NET_TYPE_UDP = 1,
    NET_TYPE_DOMAIN = 2,
    NET_TYPE_RAW = 3,
    NET_TYPE_BUTT
};

/* SOCK handle type. The value ranges from 0 to 3 (2 bits). */
enum SockHandleType {
    SOCK_HANDLE_INIT = 0,
    SOCK_HANDLE_LISTENER = 1,
    SOCK_HANDLE_CONNECTION = 2,
    SOCK_HANDLE_BUTT
};

#define SOCK_HANDLE_TYPE_SHIFT 38
#define SOCK_NET_TYPE_SHIFT    32
#define SOCK_FD_MASK           0xffffffff
#define SOCK_NET_TYPE_MASK     0x0000003f

#define SOCK_LOG_ERROR(errorCode, fmt, args...) \
    do { \
        SockErrnoSet((int)(errorCode)); \
        LogWrite(ThreadLogLevel::LOG_LEVEL_ERROR, (unsigned int)(errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)


typedef int (*SockBindHook)(SignedSocket sockFd, const struct SockAddr *addr, int backlog);

typedef int (*SockAcceptHook)(SignedSocket inFd, SignedSocket *outFd, struct SockAddr *addr,
                              unsigned long long timeout);

struct SockBufInfo {
    void *buf;
    const void *constBuf;
    unsigned int len;
};

typedef int (*SockConnectHook)(SignedSocket connFd, struct SockAddr *local,
                               struct SockAddr *peer, unsigned long long timeout);

typedef int (*SockCloseHook)(SignedSocket fd);

typedef int (*SockShutdownHook)(SignedSocket fd);

typedef int (*SockConnSendHook)(SignedSocket fd, const void *buf, unsigned int len, SocketFlag flags, int *sendLen,
                                unsigned long long timeout);
typedef int (*SockConnRecvHook)(SignedSocket fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen,
                                unsigned long long timeout);

typedef int (*SockKeepAliveSetHook)(SignedSocket fd, const struct SockKeepAliveCfg *cfg);

typedef int (*SockAddrGetHook)(SignedSocket fd, struct SockAddr *addr, bool local);

typedef int (*SockSendtoHook)(SignedSocket fd, void *bufAndLen, SocketFlag flags, const struct SockAddr *toAddr,
                              int *sendLen, unsigned long long timeout);

typedef int (*SockRecvfromHook)(SignedSocket fd, void *bufAndLen, SocketFlag flags, struct SockAddr *fromAddr,
                                int *recvLen, unsigned long long timeout);

typedef SignedSocket (*SockCreateSocketHook)(int domain, int type, int protocol, int *socketError);

typedef int (*SockOptionSetHook)(SignedSocket fd, int level, int optname,
                                 const void *optval, socklen_t optlen);

typedef int (*SockOptionGetHook)(SignedSocket fd, int level, int optname,
                                 void *optval, socklen_t *optlen);

typedef int (*SockDisconnectHook)(SignedSocket fd);

#ifdef MRT_MACOS
typedef int (*SockDisconnectForIPv6Hook)(SignedSocket fd);
#endif

typedef int (*SockSendNonBlockHook)(SignedSocket fd, const void *buf, unsigned int len,
                                    SocketFlag flags, int *sendLen);

typedef int (*SockRecvNonBlockHook)(SignedSocket fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen);

typedef int (*SockWaitSendHook)(SignedSocket fd, unsigned long long timeout);

typedef int (*SockSendtoNonBlockHook)(int fd, const void *buf, unsigned int len, SocketFlag flags,
                                      const struct SockAddr *toAddr, int *sendLen);

typedef int (*SockRecvfromNonBlockHook)(int fd, void *buf, unsigned int len, SocketFlag flags,
                                        struct SockAddr *fromAddr, int *recvLen);

typedef int (*SockWaitRecvHook)(SignedSocket fd, unsigned long long timeout);

typedef int (*SockListenHook)(SignedSocket sockFd, int backlog);

struct SockCommHooks {
    SockBindHook bind;                          /* Create and bind the local address function. If the protocol
                                                 * is connected, the listening address is required. */
    SockListenHook listen;                      /* Listening function */
    SockAcceptHook accept;                      /* Accept function */
    SockConnectHook connect;                    /* Function for creating and connecting the peer address */
    SockDisconnectHook disconnect;              /* Disconnect function */
#ifdef MRT_MACOS
    SockDisconnectForIPv6Hook disconnectForIPv6;
#endif
    SockConnSendHook send;                      /* Send message function */
    SockSendNonBlockHook sendNonBlock;          /* Send message non-block fucntion */
    SockWaitSendHook waitSend;                  /* Message sending asynchronous waiting function */
    SockConnRecvHook recv;                      /* Message receive function */
    SockRecvNonBlockHook recvNonBlock;          /* Message receive non-block function */
    SockWaitRecvHook waitRecv;                  /* Asynchronous waiting function for message receiving */
    SockCloseHook close;                        /* close fd */
    SockKeepAliveSetHook keepAliveSet;          /* set keep alive */
    SockAddrGetHook sockAddrGet;                /* Obtains the local or connected peer address bound to fd */
    SockSendtoHook sendto;                      /* Sendto, for udp and raw socket */
    SockSendtoNonBlockHook sendtoNonBlock;      /* Sendto non-block, for udp and raw socket */
    SockRecvfromHook recvfrom;                  /* Recvfrom, for udp and raw socket */
    SockRecvfromNonBlockHook recvfromNonBlock;  /* Recvfrom non-block, for udp and raw socket */
    SockCreateSocketHook createSocket;          /* create socket fuction */
    SockOptionSetHook optionSet;                /* set socket option */
    SockOptionGetHook optionGet;                /* get socket option */
    SockShutdownHook shutdown;                  /* shutdown fd */
};

int SockCommHooksReg(SockNetType type, const struct SockCommHooks *hooks);

/**
 * @brief tcp、udp、raw SockAddrGet general function
 * @param fd            [IN] fd
 * @param addr          [IN] address
 * @param local         [IN] server/client
 * @retval #0 or error code
 */
int SockAddrGetGeneral(SignedSocket fd, struct SockAddr *addr, bool local);

/**
 * @brief setsockopt general function
 * @param fd            [IN] sock fd
 * @param level         [IN] level
 * @param optname       [IN] option name
 * @param optval        [IN] option value
 * @param optlen        [IN] option length
 * @retval #0 or -1
 */
int SockOptionSetGeneral(SignedSocket fd, int level, int optname, const void *optval, socklen_t optlen);

/**
 * @brief getsockopt general function
 * @param fd            [IN] sock fd
 * @param level         [IN] level
 * @param optname       [IN] option name
 * @param optval        [IN] option value
 * @param optlen        [IN] option length
 * @retval #0 or -1
 */
int SockOptionGetGeneral(SignedSocket fd, int level, int optname, void *optval, socklen_t *optlen);

/**
 * @brief tcp、udp、raw SockClose general function
 * @param fd            [OUT] fd
 * @retval #0 or error code
 */
int SockCloseGeneral(SignedSocket fd);

/**
 * @brief tcp、udp、raw SockShutdown general function
 * @param fd            [OUT] fd
 * @retval #0 or error code
 */
int SockShutdownGeneral(SignedSocket fd);

/**
 * @brief  SocketRecv general function
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buffer lenth
 * @param flags         [IN]  flags
 * @param recvLen       [OUT] length of the received message
 * @param timeout       [IN] timeout
 * @retval #0 or error code
 */
int SockRecvGeneral(SignedSocket fd, void *buf, unsigned int len, SocketFlag flags,
                    int *recvLen, unsigned long long timeout);

/**
 * @brief SocketSend general function
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buffer lenth
 * @param flags         [IN]  flags
 * @param sendLen       [OUT] length of the sent message
 * @param timeout       [IN] timeout
 * @retval #0 or error code
 */
int SockSendGeneral(SignedSocket fd, const void *buf, unsigned int len, SocketFlag flags,
                    int *sendLen, unsigned long long timeout);

/**
 * @brief SocketSendto
 * @param fd            [IN] fd
 * @param bufAndLen     [IN] buffer info
 * @param flags         [IN]  flags
 * @param toAddr        [IN] address
 * @param sendLen       [OUT] length of the sent message
 * @param timeout       [IN] timeout
 * @retval #0 or error code
 */
int SockSendtoGeneral(SignedSocket fd, void *bufAndLen, SocketFlag flags, const struct SockAddr *toAddr,
                      int *sendLen, unsigned long long timeout);

/**
 * @brief SocketRecvfrom
 * @param fd            [IN] fd
 * @param bufAndLen     [IN] buffer info
 * @param flags         [IN]  flags
 * @param toAddr        [IN] address
 * @param recvLen       [OUT] length of the received message
 * @param timeout       [IN] timeout
 * @retval #0 or error code
 */
int SockRecvfromGeneral(SignedSocket fd, void *bufAndLen, SocketFlag flags, struct SockAddr *fromAddr,
                        int *recvLen, unsigned long long timeout);

/**
 * @brief SocketSend non-block
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buf length
 * @param flags         [IN]  flags
 * @param sendLen       [OUT] length of the sent message
 * @retval #0 or error code
 */
int SockSendNonBlockGeneral(int fd, const void *buf, unsigned int len, SocketFlag flags, int *sendLen);

/**
 * @brief  SocketRecv non-block
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buffer length
 * @param flags         [IN]  flags
 * @param recvLen       [OUT] length of the received message
 * @retval #0 or error code
 */
int SockRecvNonBlockGeneral(SignedSocket fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen);

/**
 * @brief SocketSendto non-block
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buffer length
 * @param flags         [IN]  flags
 * @param toAddr        [IN] address
 * @param sendLen       [OUT] length of the sent message
 * @retval #0 or error code
 */
int SockSendtoNonBlockGeneral(SignedSocket fd, const void *buf, unsigned int len, SocketFlag flags,
                              const struct SockAddr *toAddr, int *sendLen);

/**
 * @brief Recvfrom non-block
 * @param fd            [IN] fd
 * @param buf           [IN] buffer
 * @param len           [IN] buffer length
 * @param flags         [IN]  flags
 * @param fromAddr      [IN] address
 * @param recvLen       [OUT] length of the received message
 * @retval #0 or error code
 */
int SockRecvfromNonBlockGeneral(SignedSocket fd, void *buf, unsigned int len, SocketFlag flags,
                                struct SockAddr *fromAddr, int *recvLen);

/**
 * @brief when windows socket connect and bind is NULL, create default address
 * @param family        [OUT] family
 * @param addr          [IN] SockAddr
 * @retval #0 or error code
 */
int SockGetLocalDefaultAddr(short family, struct SockAddr *addr);

#if defined (MRT_LINUX) || defined (MRT_MACOS)

int SockCreateInternal(int domain, int type, int protocol, int *socketError);
int SockAcceptInternal(int inFd, struct sockaddr *sockaddr, socklen_t *addrLen, int &globalErr, int &fcntlErr);

#endif

#ifdef MRT_WINDOWS

struct LpfnMswsockHooks {
    LPFN_CONNECTEX ConnectEx;
    LPFN_ACCEPTEX AcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs;
};

extern struct LpfnMswsockHooks g_mswsockHooks;

/**
 * @brief Init socket on windows
 * @retval #0 or error code
 */
int SockWinStartup(void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_SOCK_IMPL_H */
