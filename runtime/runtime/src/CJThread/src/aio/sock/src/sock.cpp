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


#include <cstring>
#include <cstdint>
#include <cerrno>
#include "schedule_impl.h"
#include "securec.h"
#include "sock_impl.h"
#include "macro_def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SockCommHooks g_sockCommHooks[NET_TYPE_BUTT];
const int SHUTDOWN_WAIT_TIME = 1000;
#ifdef MRT_WINDOWS
struct LpfnMswsockHooks g_mswsockHooks;
#endif

void SockErrnoSet(int err)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return;
    }
    codethread->coErrno = err;
}

int SockErrnoGet(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return 0;
    }
    return codethread->coErrno;
}

/* The internal interface does not determine the type range or
 * null pointer, and the caller ensures the validity of the parameters.
 */
int SockCommHooksReg(SockNetType type, const struct SockCommHooks *hooks)
{
    int ret;

    ret = memcpy_s(&g_sockCommHooks[type], sizeof(struct SockCommHooks), hooks, sizeof(struct SockCommHooks));
    if (ret != EOK) {
        SOCK_LOG_ERROR(ret, "memcpy failed");
        return ret;
    }

    return 0;
}

SockNetType SockStrToNetType(const char *net)
{
    if (net == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg is nullptr");
        return NET_TYPE_BUTT;
    }

    if (strncmp(net, "tcp", strlen("tcp")) == 0) {
        return NET_TYPE_TCP;
    }

    if (strncmp(net, "udp", strlen("udp")) == 0) {
        return NET_TYPE_UDP;
    }

    if (strncmp(net, "raw", strlen("raw")) == 0) {
        return NET_TYPE_RAW;
    }

    if (strncmp(net, "domain", strlen("domain")) == 0) {
        return NET_TYPE_DOMAIN;
    }

    SOCK_LOG_ERROR(ERRNO_SOCK_NOT_SUPPORTED, "net unsupported: %s", net);
    return NET_TYPE_BUTT;
}

/* Windows raw_fd is an unsigned long, and since raw_fd<=SOCK_FD_MAX_NUM,
 * type casting will not be truncated here.
 */
MRT_STATIC_INLINE long long SockHandleMake(unsigned int rawFd, unsigned int netType,
                                           unsigned int handleType)
{
    return (long long)(((unsigned long long)handleType << SOCK_HANDLE_TYPE_SHIFT) |
                       (((unsigned long long)netType & SOCK_NET_TYPE_MASK) << SOCK_NET_TYPE_SHIFT) |
                       (unsigned long long)rawFd);
}

MRT_STATIC_INLINE SignedSocket SockFdGet(unsigned long long handle)
{
    return (SignedSocket)(handle & SOCK_FD_MASK);
}

MRT_STATIC_INLINE unsigned int SockNetTypeGet(unsigned long long handle)
{
    return (unsigned int)((handle >> SOCK_NET_TYPE_SHIFT) & SOCK_NET_TYPE_MASK);
}

MRT_STATIC_INLINE unsigned int SockHandleTypeGet(unsigned long long handle)
{
    return (unsigned int)(handle >> SOCK_HANDLE_TYPE_SHIFT);
}

int SockHandleParse(long long sock, SockHandleType expect, unsigned int *netType, SignedSocket *fd)
{
    unsigned int nType;

    nType = SockNetTypeGet((unsigned long long)sock);
    if (nType >= NET_TYPE_BUTT) {
        SOCK_LOG_ERROR(ERRNO_SOCK_HANDLE_INVALID, "netType invalid, sock: 0x%llx", sock);
        return -1;
    }

    if ((expect != SOCK_HANDLE_BUTT) && (SockHandleTypeGet((unsigned long long)sock) != expect)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_HANDLE_INVALID, "handle_type invalid, sock: 0x%llx, expect: %u", sock, expect);
        return -1;
    }

    *netType = nType;
    *fd = SockFdGet((unsigned long long)sock);
    return 0;
}

long long SockCreate(int domain, int type, int protocol, const char *net)
{
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    unsigned int netType;
    unsigned int handleType;
    long long sock;
    int socketError;

    netType = SockStrToNetType(net);
    if (netType >= NET_TYPE_BUTT) {
        return -1;
    }

    handleType = (type == SOCK_STREAM) ? SOCK_HANDLE_INIT : SOCK_HANDLE_CONNECTION;
    sockHooks = &g_sockCommHooks[netType];

    if (sockHooks->createSocket != nullptr) {
        rawFd = sockHooks->createSocket(domain, type, protocol, &socketError);
#ifdef MRT_WINDOWS
        if (rawFd == INVALID_SOCKET) {
            SOCK_LOG_ERROR(socketError, "createSocket failed, net: %s, domain: %d, type: %d",
                           net, domain, type);
            return -1;
        }
#else
        if (rawFd == -1) {
            SOCK_LOG_ERROR(socketError, "createSocket failed, net: %s, domain: %d, type: %d",
                           net, domain, type);
            return -1;
        }
#endif
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "createSocket is nullptr, net: %s", net);
        return -1;
    }

    sock = SockHandleMake((unsigned int)rawFd, netType, handleType);
    LOG_INFO(0, "createSocket success, fd: %u, net: %s, sock: 0x%llx", (unsigned int)rawFd, net, sock);
    return sock;
}

long long SockBind(long long sock, struct SockAddr *addr, int backlog)
{
    struct SockCommHooks *sockHooks;
    long long bindSock;
    unsigned int netType;
    unsigned int handleType;
    SignedSocket rawFd;
    int ret;

    if (addr == nullptr || addr->sockaddr == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "addr is nullptr");
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    handleType = (SockHandleTypeGet((unsigned long long)sock) == SOCK_HANDLE_CONNECTION) ?
                 SOCK_HANDLE_CONNECTION : SOCK_HANDLE_LISTENER;

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->bind != nullptr) {
        ret = sockHooks->bind(rawFd, addr, backlog);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "bind failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "bind is nullptr, sock: 0x%llx", sock);
        return -1;
    }
    bindSock = SockHandleMake((unsigned int)rawFd, netType, handleType);
    LOG_INFO(0, "SockBind success, fd: %u, bindSock: 0x%llx", (unsigned int)rawFd, bindSock);
    return bindSock;
}

long long SockListen(long long sock, int backlog)
{
    struct SockCommHooks *sockHooks;
    long long listenSock;
    unsigned int netType;
    unsigned int handleType;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    handleType = SOCK_HANDLE_LISTENER;

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->listen != nullptr) {
        ret = sockHooks->listen(rawFd, backlog);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "listen failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "listen is nullptr, sock: 0x%llx", sock);
        return -1;
    }
    listenSock = SockHandleMake((unsigned int)rawFd, netType, handleType);
    LOG_INFO(0, "SockListen success, fd: %u, listenSock: 0x%llx", (unsigned int)rawFd, listenSock);
    return listenSock;
}

long long SockAcceptTimeout(long long sock, struct SockAddr *addr, unsigned long long timeout)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket outFd;
    SignedSocket inFd;
    int ret;
    long long outSock;

    if (SockHandleParse(sock, SOCK_HANDLE_LISTENER, &netType, &inFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->accept != nullptr) {
        ret = sockHooks->accept(inFd, &outFd, addr, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "accept failed, sock: 0x%llx", sock);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    outSock = SockHandleMake((unsigned int)outFd, netType, SOCK_HANDLE_CONNECTION);
    LOG_INFO(0, "SockAccept success, sock: 0x%llx, accept fd:%u, accept sock: 0x%llx", sock, outFd, outSock);
    return outSock;
}

long long SockAccept(long long sock, struct SockAddr *addr)
{
    return SockAcceptTimeout(sock, addr, (unsigned long long) -1);
}

long long SockConnectTimeout(long long sock, struct SockAddr *localAddr,
                             struct SockAddr *peerAddr, unsigned long long timeout)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    long long connSock;
    int ret;
    SignedSocket rawFd;

    if (timeout == 0) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "timeout cannot be zero");
        return -1;
    }
    if (peerAddr == nullptr || peerAddr->sockaddr == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "peerAddr is nullptr");
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->connect == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "connect is nullptr, netType: %u", netType);
        return -1;
    }
    ret = sockHooks->connect(rawFd, localAddr, peerAddr, timeout);
    if (ret != 0) {
        if (ret == ERRNO_SCHDFD_TIMEOUT) {
            SockErrnoSet(ERRNO_SOCK_TIMEOUT);
        } else {
            SOCK_LOG_ERROR(ret, "connect failed, sock: 0x%llx", sock);
        }
        return -1;
    }

    connSock = SockHandleMake((unsigned int)rawFd, netType, SOCK_HANDLE_CONNECTION);
    LOG_INFO(0, "SockConnect success, fd: %d, connect sock: 0x%llx", rawFd, connSock);
    return connSock;
}

long long SockConnect(long long sock, struct SockAddr *localAddr, struct SockAddr *peerAddr)
{
    return SockConnectTimeout(sock, localAddr, peerAddr, (unsigned long long) -1);
}

int SockDisconnect(long long sock, bool isIPv6)
{
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    unsigned int netType;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    
#ifdef MRT_MACOS
    if (isIPv6) {
        if (sockHooks->disconnectForIPv6 != nullptr) {
            ret = sockHooks->disconnectForIPv6(rawFd);
            if (ret != 0) {
                SOCK_LOG_ERROR(ret, "ipv6 disconnect failed, sock: 0x%llx", sock);
                return -1;
            }
        } else {
            SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to func, sock: 0x%llx", sock);
            return -1;
        }
        LOG_INFO(0, "SockDisconnect success, sock: 0x%llx", sock);
        return 0;
    }
#endif
    (void)isIPv6;
    if (sockHooks->disconnect != nullptr) {
        ret = sockHooks->disconnect(rawFd);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "disconnect failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockDisconnect success, sock: 0x%llx", sock);
    return 0;
}

int SockSendTimeout(long long sock, const void *buf, unsigned int len, SocketFlag flags, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    SocketFlag sendFlag = flags;
    int ret;
    int sendLen;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
#ifdef MRT_WINDOWS
        sendFlag = 0;
#else
        sendFlag = MSG_NOSIGNAL;
#endif
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->send != nullptr) {
        ret = sockHooks->send(rawFd, buf, len, sendFlag, &sendLen, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "send failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockSend success, sock: 0x%llx, msg len: %u, send len: %u", sock, len, sendLen);
    return sendLen;
}

int SockSend(long long sock, const void *buf, unsigned int len, SocketFlag flags)
{
    return SockSendTimeout(sock, buf, len, flags, (unsigned long long) -1);
}

int SockSendNonBlock(long long sock, const void *buf, unsigned int len, SocketFlag flags)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    SocketFlag sendNonBlockFlag = flags;
    int sendLen;
    int ret;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
#ifdef MRT_WINDOWS
        sendNonBlockFlag = 0;
#else
        sendNonBlockFlag = MSG_NOSIGNAL;
#endif
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->sendNonBlock != nullptr) {
        ret = sockHooks->sendNonBlock(rawFd, buf, len, sendNonBlockFlag, &sendLen);
        if (ret != 0) {
            // EAGAIN only returns an error without logging.
            if (ret == EAGAIN) {
                SockErrnoSet(ERRNO_SOCK_EAGAIN);
            } else {
                SOCK_LOG_ERROR(ret, "send failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockSendNonBlock success, sock: 0x%llx, msg len: %u, send len: %u", sock, len, sendLen);
    return sendLen;
}

int SockWaitSendTimeout(long long sock, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->waitSend != nullptr) {
        ret = sockHooks->waitSend(rawFd, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "send wait failed, sock: 0x%llx", sock);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "send event ready, sock: 0x%llx", sock);
    return 0;
}

int SockWaitSend(long long sock)
{
    return SockWaitSendTimeout(sock, (unsigned long long)-1);
}

int SockRecvTimeout(long long sock, void *buf, unsigned int len, SocketFlag flags, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    SocketFlag recvFlag = flags;
    int ret;
    int recvLen;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        recvFlag = 0;
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->recv != nullptr) {
        ret = sockHooks->recv(rawFd, buf, len, recvFlag, &recvLen, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "recv failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockRecv success, sock: 0x%llx, recv len: %u", sock, recvLen);
    return recvLen;
}

int SockRecv(long long sock, void *buf, unsigned int len, SocketFlag flags)
{
    return SockRecvTimeout(sock, buf, len, flags, (unsigned long long) -1);
}

int SockRecvNonBlock(long long sock, void *buf, unsigned int len, SocketFlag flags)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    SocketFlag recvNonBlockFlag = flags;
    int recvLen;
    int ret;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        recvNonBlockFlag = 0;
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->recvNonBlock != nullptr) {
        ret = sockHooks->recvNonBlock(rawFd, buf, len, recvNonBlockFlag, &recvLen);
        if (ret != 0) {
            // EAGAIN only returns an error without logging.
            if (ret == EAGAIN) {
                SockErrnoSet(ERRNO_SOCK_EAGAIN);
            } else {
                SOCK_LOG_ERROR(ret, "recv failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockRecvNonBlock success, sock: 0x%llx, recv len: %u", sock, recvLen);
    return recvLen;
}

int SockWaitRecvTimeout(long long sock, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->waitRecv != nullptr) {
        ret = sockHooks->waitRecv(rawFd, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "recv wait failed, sock: 0x%llx", sock);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "Recv event ready, sock: 0x%llx", sock);
    return 0;
}

int SockWaitRecv(long long sock)
{
    return SockWaitRecvTimeout(sock, (unsigned long long)-1);
}

int SockClose(long long sock)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->close != nullptr) {
        ret = sockHooks->close(rawFd);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "close failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockClose success, sock: 0x%llx", sock);
    return 0;
}

int SockShutdown(long long sock)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->shutdown != nullptr) {
        ret = sockHooks->shutdown(rawFd);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "shutdown failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }
    LOG_INFO(0, "SockShutdown success, sock: 0x%llx", sock);
    return 0;
}

int SockKeepAliveSet(long long sock, const struct SockKeepAliveCfg *cfg)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (cfg == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid");
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->keepAliveSet != nullptr) {
        ret = sockHooks->keepAliveSet(rawFd, cfg);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "keepAliveSet failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return 0;
}

int SockAddrGet(long long sock, struct SockAddr *addr, bool local)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (addr == nullptr || addr->sockaddr == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid");
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->sockAddrGet != nullptr) {
        ret = sockHooks->sockAddrGet(rawFd, addr, local);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "sockAddrGet failed, sock: 0x%llx", sock);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return 0;
}

int SockLocalAddrGet(long long sock, struct SockAddr *addr)
{
    return SockAddrGet(sock, addr, true);
}

int SockPeerAddrGet(long long sock, struct SockAddr *addr)
{
    return SockAddrGet(sock, addr, false);
}

int SockSendtoTimeout(long long sock, const void *buf, unsigned int len, SocketFlag flags,
                      const struct SockAddr *addr, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    struct SockBufInfo bufInfo;
    SignedSocket rawFd;
    SocketFlag sendtoFlag = flags;
    int ret;
    int sendLen;

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }

    if (addr == nullptr || addr->sockaddr == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "addr is nullptr");
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        sendtoFlag = 0;
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->sendto != nullptr) {
        bufInfo.constBuf = buf;
        bufInfo.len = len;
        ret = sockHooks->sendto(rawFd, &bufInfo, sendtoFlag, addr, &sendLen, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "sendto failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return sendLen;
}

int SockSendto(long long sock, const void *buf, unsigned int len, SocketFlag flags, const struct SockAddr *addr)
{
    return SockSendtoTimeout(sock, buf, len, flags, addr, (unsigned long long)-1);
}

int SockSendtoNonBlock(long long sock, const void *buf, unsigned int len, SocketFlag flags,
                       const struct SockAddr *addr)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    SignedSocket rawFd;
    SocketFlag sendtoNonBlockFlag = flags;
    int sendLen;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (addr == nullptr || addr->sockaddr == nullptr) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "addr is nullptr");
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        sendtoNonBlockFlag = 0;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->sendtoNonBlock != nullptr) {
        ret = sockHooks->sendtoNonBlock(rawFd, buf, len, sendtoNonBlockFlag, addr, &sendLen);
        if (ret != 0) {
            // EAGAIN only returns an error without logging.
            if (ret == EAGAIN) {
                SockErrnoSet(ERRNO_SOCK_EAGAIN);
            } else {
                SOCK_LOG_ERROR(ret, "sendtoNonBlock failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sendtoNonBlock is nullptr, sock: 0x%llx", sock);
        return -1;
    }

    return sendLen;
}

int SockRecvfromTimeout(long long sock, void *buf, unsigned int len, SocketFlag flags,
                        struct SockAddr *addr, unsigned long long timeout)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    struct SockBufInfo bufInfo;
    int ret;
    int recvLen;
    SocketFlag recvfromFlag = flags;
    SignedSocket rawFd;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        recvfromFlag = 0;
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->recvfrom != nullptr) {
        bufInfo.buf = buf;
        bufInfo.len = len;
        ret = sockHooks->recvfrom(rawFd, &bufInfo, recvfromFlag, addr, &recvLen, timeout);
        if (ret != 0) {
            if (ret == ERRNO_SCHDFD_TIMEOUT) {
                SockErrnoSet(ERRNO_SOCK_TIMEOUT);
            } else {
                SOCK_LOG_ERROR(ret, "recvfrom failed, sock: 0x%llx, len: %u", sock);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return recvLen;
}

int SockRecvfrom(long long sock, void *buf, unsigned int len, SocketFlag flags, struct SockAddr *addr)
{
    return SockRecvfromTimeout(sock, buf, len, flags, addr, (unsigned long long)-1);
}

int SockRecvfromNonBlock(long long sock, void *buf, unsigned int len, SocketFlag flags, struct SockAddr *addr)
{
    unsigned int netType;
    struct SockCommHooks *sockHooks;
    int recvLen;
    int ret;
    SocketFlag recvfromNonBlockFlag = flags;
    SignedSocket rawFd;

    if ((buf == nullptr) || (len == 0)) {
        SOCK_LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "arg invalid, len: %u", len);
        return -1;
    }

    if (SockHandleParse(sock, SOCK_HANDLE_CONNECTION, &netType, &rawFd) != 0) {
        return -1;
    }
    if (netType != NET_TYPE_RAW) {
        recvfromNonBlockFlag = 0;
    }
    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->recvfromNonBlock != nullptr) {
        ret = sockHooks->recvfromNonBlock(rawFd, buf, len, recvfromNonBlockFlag, addr, &recvLen);
        if (ret != 0) {
            // EAGAIN only returns an error without logging.
            if (ret == EAGAIN) {
                SockErrnoSet(ERRNO_SOCK_EAGAIN);
            } else {
                SOCK_LOG_ERROR(ret, "recvfromNonBlock failed, sock: 0x%llx, len: %u", sock, len);
            }
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "recvfromNonBlock is nullptr, sock: 0x%llx", sock);
        return -1;
    }

    return recvLen;
}

int SockOptionSet(long long sock, int level, int optname, const void *optval, int optlen)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->optionSet != nullptr) {
        ret = sockHooks->optionSet(rawFd, level, optname, optval, (socklen_t)optlen);
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "setopt failed, sock: 0x%llx, level: %d, optname: %d", sock, level, optname);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return 0;
}

int SockOptionGet(long long sock, int level, int optname, void *optval, int *optlen)
{
    struct SockCommHooks *sockHooks;
    unsigned int netType;
    SignedSocket rawFd;
    int ret;

    if (SockHandleParse(sock, SOCK_HANDLE_BUTT, &netType, &rawFd) != 0) {
        return -1;
    }

    sockHooks = &g_sockCommHooks[netType];
    if (sockHooks->optionGet != nullptr) {
        ret = sockHooks->optionGet(rawFd, level, optname, optval, reinterpret_cast<socklen_t *>(optlen));
        if (ret != 0) {
            SOCK_LOG_ERROR(ret, "getopt failed, sock: 0x%llx, level: %d, optname: %d", sock, level, optname);
            return -1;
        }
    } else {
        SOCK_LOG_ERROR(ERRNO_SOCK_NOT_REGISTERED, "sock invalid to the func, sock: 0x%llx", sock);
        return -1;
    }

    return 0;
}

int SockAddrGetGeneral(SignedSocket fd, struct SockAddr *addr, bool local)
{
    int ret;

    if (local) {
        ret = getsockname(fd, reinterpret_cast<struct sockaddr *>(addr->sockaddr), &addr->addrLen);
    } else {
        ret = getpeername(fd, reinterpret_cast<struct sockaddr *>(addr->sockaddr), &addr->addrLen);
    }

    if (ret != 0)  {
#ifdef MRT_WINDOWS
        ret = WSAGetLastError();
        LOG_ERROR(ret, "get sockaddr failed, fd: %llu, is local: %d", fd, local);
        return ret;
#else
        ret = errno;
        LOG_ERROR(ret, "get sockaddr failed, fd: %d, is local: %d", fd, local);
        return ret;
#endif
    }
    return 0;
}

int SockOptionSetGeneral(SignedSocket fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int ret;

#ifdef MRT_WINDOWS
    if (setsockopt(fd, level, optname, static_cast<const char *>(optval), optlen) == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "setsockopt failed, fd: %d, level: %d, optname: %d", fd, level, optname);
        return ret;
    }
#else
    if (setsockopt(fd, level, optname, optval, optlen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "setsockopt failed, fd: ld, level: %d, optname: %d", fd, level, optname);
        return ret;
    }
#endif

    return 0;
}

int SockOptionGetGeneral(SignedSocket fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int ret;

#ifdef MRT_WINDOWS
    if (getsockopt(fd, level, optname, static_cast<char *>(optval), (int *)optlen) == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "getsockopt failed, fd: %d, level: %d, optname: %d", fd, level, optname);
        return ret;
    }
#else
    if (getsockopt(fd, level, optname, optval, optlen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "getsockopt failed, fd: %d, level: %d, optname: %d", fd, level, optname);
        return ret;
    }
#endif
    return 0;
}

int SockCloseGeneral(SignedSocket fd)
{
    int ret;

    ret = SchdfdDeregister(fd);
    if (ret != 0) {
        return ret;
    }
#ifdef MRT_WINDOWS
    ret = closesocket(fd);
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "close failed, fd: %d", fd);
        return ret;
    }
#else
    ret = close(fd);
    if (ret == -1) {
        ret = errno;
        LOG_ERROR(ret, "close failed, fd: %d", fd);
        return ret;
    }
#endif

    return 0;
}

int SockShutdownGeneral(SignedSocket fd)
{
    int ret;
    WakeallFd(fd);
#ifdef MRT_WINDOWS
    ret = shutdown(fd, SD_BOTH);
    if (ret == -1) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "shutdown failed, fd: %d", fd);
        return ret;
    }
#else
    ret = shutdown(fd, SHUT_RDWR);
    if (ret == -1) {
        ret = errno;
        LOG_ERROR(ret, "shutdown failed, fd: %d", fd);
        return ret;
    }
#endif
    // sleep to resolve timer memory leak.
    usleep(SHUTDOWN_WAIT_TIME);
    return 0;
}

#ifdef MRT_WINDOWS
int SockWinStartup(void)
{
    int ret;
    WORD versionRequired;
    WSADATA wsadata;

    // Set Windows Socket Version 2.2
    versionRequired = MAKEWORD(2, 2);
    // The use of Winsock DLL by the startup process.
    ret = WSAStartup(versionRequired, &wsadata);
    if (ret != 0) {
        LOG_ERROR(ret, "winsock startup failed");
        return ret;
    }
    return 0;
}

int SockLoadMswsockHooks(SOCKET fd, void *func, GUID *guid)
{
    int ret;
    DWORD n;

    if (func == nullptr) {
        return ERRNO_SOCK_ARG_INVALID;
    }

    // Socket mode control function, used here to obtain function pointers for specific functions.
    ret = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, guid,
                   sizeof(GUID), func, sizeof(void *),
                   &n, nullptr, nullptr);
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "WSAIoctl get Lpfn mswsock hooks failed");
        return ret;
    }
    return 0;
}

/* The Windows asynchronous interfaces WSASend and WSARecv can be used directly.
 * Interfaces such as ConnectEx, AcceptEx, and MyAcceptExSockaddrs require calling WSAIontl function to obtain.
 */
int SockMswsockHooksReg()
{
    int ret;
    SOCKET fd;
    GUID guidConnectex = WSAID_CONNECTEX;
    GUID guidAcceptex = WSAID_ACCEPTEX;
    GUID guidGetAcceptexSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

    fd = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr,
                   0, WSA_FLAG_OVERLAPPED);
    if (fd == INVALID_SOCKET) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "socket failed, domain: %u, protocol: %d", AF_INET, 0);
        return ret;
    }

    ret = SockLoadMswsockHooks(fd, &g_mswsockHooks.ConnectEx, &guidConnectex);
    if (ret != 0) {
        (void)closesocket(fd);
        return ret;
    }
    ret = SockLoadMswsockHooks(fd, &g_mswsockHooks.AcceptEx, &guidAcceptex);
    if (ret != 0) {
        (void)closesocket(fd);
        return ret;
    }
    ret = SockLoadMswsockHooks(fd, &g_mswsockHooks.GetAcceptExSockaddrs, &guidGetAcceptexSockaddrs);
    if (ret != 0) {
        (void)closesocket(fd);
        return ret;
    }

    (void)closesocket(fd);
    return 0;
}

__attribute__((constructor)) void SockWinInit(void)
{
    int ret;

    // init winsock.
    ret = SockWinStartup();
    if (ret != 0) {
        LOG_ERROR(ret, "SockWinStartup failed!");
    }
    // get asynchronous operation function pointer
    ret = SockMswsockHooksReg();
    if (ret != 0) {
        LOG_ERROR(ret, "SockMswsockHooksReg failed!");
    }
    /* Check if the system supports the function of immediately completing IO operations
     * without adding them to the waiting queue. If supported, the asynchronous operation
     * will immediately return success and will no longer be added to the IOCP waiting queue.
     * The function will immediately return success. If not supported, the asynchronous
     * operation will be added to the waiting queue immediately after completion,
     * and the codethread will be blocked and later awakened by schdpoll.
     * */
    SchdfdCheckIocpCompleteSkip();
}

int SockSendGeneral(SOCKET fd, const void *buf, unsigned int len, SocketFlag flags,
                    int *sendLen, unsigned long long timeout)
{
    int ret;
    int sendRet;
    SchdpollEventType type = SHCDPOLL_WRITE;
    struct IocpOperation *operation;

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    operation = SchdfdUpdateIocpOperationInlock(fd, type, buf, len);
    if (operation == nullptr) {
        ret = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ret, "get iocp write operation failed , fd: %llu", fd);
        return ret;
    }

    // Asynchronous send operation for delivery, sending data, asynchronous interface
    // will immediately return.
    sendRet = WSASend(fd, &operation->iocpBuf, 1, &operation->transBytes,
                      flags, &operation->overlapped, nullptr);
    if (sendRet == 0 && SchdfdUseSkipIocp()) {
        *sendLen = static_cast<int>(operation->transBytes);
        SchdfdUnlock(fd, type);
        return 0;
    }

    if (sendRet == SOCKET_ERROR) {
        ret = WSAGetLastError();
        if (ret != WSA_IO_PENDING) {
            LOG_ERROR(ret, "send failed, fd: %llu, len: %u", fd, len);
            SchdfdUnlock(fd, type);
            return ret;
        }
    }

    ret = SchdfdIocpWaitInlock(operation, timeout);
    if (ret != 0) {
        SchdfdUnlock(fd, type);
        return ret;
    }

    *sendLen = static_cast<int>(operation->transBytes);
    SchdfdUnlock(fd, type);
    return 0;
}

int SockRecvGeneral(SOCKET fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen, unsigned long long timeout)
{
    int ret;
    int recvRet;
    SchdpollEventType type = SHCDPOLL_READ;
    struct IocpOperation *operation;
    DWORD lpFlags = flags;

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    operation = SchdfdUpdateIocpOperationInlock(fd, type, buf, len);
    if (operation == nullptr) {
        ret = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ret, "update iocp read operation failed , fd: %llu", fd);
        return ret;
    }

    // Submit asynchronous recv operation, accept data, asynchronous interface
    // will immediately return.
    recvRet = WSARecv(fd, &operation->iocpBuf, 1, &operation->transBytes,
                      &lpFlags, &operation->overlapped, nullptr);
    if (recvRet == 0 && SchdfdUseSkipIocp()) {
        *recvLen = static_cast<int>(operation->transBytes);
        SchdfdUnlock(fd, type);
        return 0;
    }

    if (recvRet == SOCKET_ERROR) {
        ret = WSAGetLastError();
        if (ret != WSA_IO_PENDING) {
            LOG_ERROR(ret, "recv failed, fd: %llu, len: %u", fd, len);
            SchdfdUnlock(fd, type);
            return ret;
        }
    }

    ret = SchdfdIocpWaitInlock(operation, timeout);
    if (ret != 0) {
        SchdfdUnlock(fd, type);
        return ret;
    }

    *recvLen = static_cast<int>(operation->transBytes);
    SchdfdUnlock(fd, type);
    return 0;
}

int SockSendtoGeneral(SOCKET fd, void *bufAndLen, SocketFlag flags, const struct SockAddr *toAddr, int *sendLen,
                      unsigned long long timeout)
{
    int err;
    int sendErr;
    struct SockBufInfo *bufInfo = static_cast<struct SockBufInfo *>(bufAndLen);
    SchdpollEventType pollType = SHCDPOLL_WRITE;
    struct IocpOperation *ioperation;

    err = SchdfdLock(fd, pollType);
    if (err != 0) {
        return err;
    }

    ioperation = SchdfdUpdateIocpOperationInlock(fd, pollType, bufInfo->constBuf, bufInfo->len);
    if (ioperation == nullptr) {
        err = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(err, "get iocp write operation failed , fd: %llu", fd);
        return err;
    }

    // Asynchronous sendTo operation for delivery, sending data, asynchronous interface
    // will immediately return.
    sendErr = WSASendTo(fd, &ioperation->iocpBuf, 1, &ioperation->transBytes, flags,
                        reinterpret_cast<struct sockaddr *>(toAddr->sockaddr), toAddr->addrLen,
                        &ioperation->overlapped, nullptr);
    if (sendErr == 0 && SchdfdUseSkipIocp()) {
        *sendLen = static_cast<int>(ioperation->transBytes);
        SchdfdUnlock(fd, pollType);
        return 0;
    }

    if (sendErr == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            LOG_ERROR(err, "send failed, fd: %llu, len: %u", fd, bufInfo->len);
            SchdfdUnlock(fd, pollType);
            return err;
        }
    }

    err = SchdfdIocpWaitInlock(ioperation, timeout);
    if (err != 0) {
        SchdfdUnlock(fd, pollType);
        return err;
    }

    *sendLen = static_cast<int>(ioperation->transBytes);
    SchdfdUnlock(fd, pollType);
    return 0;
}

int SockRecvfromGeneral(SOCKET fd, void *bufAndLen, SocketFlag flags, struct SockAddr *fromAddr,
                        int *recvLen, unsigned long long timeout)
{
    int err;
    int recvErr;
    socklen_t addrLen = 0;
    SchdpollEventType pollType = SHCDPOLL_READ;
    struct SockBufInfo *bufInfo = static_cast<struct SockBufInfo *>(bufAndLen);
    struct IocpOperation *ioperation;
    struct sockaddr *sockAddr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ?
                                nullptr : reinterpret_cast<struct sockaddr *>(fromAddr->sockaddr);
    socklen_t *addrLenPtr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ? &addrLen : &(fromAddr->addrLen);
    DWORD lpFlags = flags;
    err = SchdfdLock(fd, pollType);
    if (err != 0) {
        return err;
    }

    ioperation = SchdfdUpdateIocpOperationInlock(fd, pollType, bufInfo->buf, bufInfo->len);
    if (ioperation == nullptr) {
        err = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(err, "update iocp read operation failed , fd: %llu", fd);
        return err;
    }

    // Submit asynchronous recvFrom operation, accept data, asynchronous interface
    // will immediately return.
    recvErr = WSARecvFrom(fd, &ioperation->iocpBuf, 1, &ioperation->transBytes,
                          &lpFlags, sockAddr, addrLenPtr, &ioperation->overlapped, nullptr);
    if (recvErr == 0 && SchdfdUseSkipIocp()) {
        err = 0;
        *recvLen = static_cast<int>(ioperation->transBytes);
        SchdfdUnlock(fd, pollType);
        return err;
    }

    if (recvErr == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            LOG_ERROR(err, "recv failed, fd: %llu, len: %u", fd, bufInfo->len);
            SchdfdUnlock(fd, pollType);
            return err;
        }
    }

    err = SchdfdIocpWaitInlock(ioperation, timeout);
    if (err != 0) {
        SchdfdUnlock(fd, pollType);
        return err;
    }

    err = 0;
    *recvLen = static_cast<int>(ioperation->transBytes);
    SchdfdUnlock(fd, pollType);
    return err;
}

int SockGetLocalDefaultAddr(short family, struct SockAddr *addr)
{
    struct sockaddr_in *ipv4Addr;
    struct sockaddr_in6 *ipv6Addr;
    switch (family) {
        case AF_INET:
            ipv4Addr = reinterpret_cast<struct sockaddr_in *>(addr->sockaddr);
            ipv4Addr->sin_family = AF_INET;
            ipv4Addr->sin_port = 0;
            ipv4Addr->sin_addr.s_addr = INADDR_ANY;
            addr->addrLen = static_cast<socklen_t>(sizeof(struct sockaddr_in));
            break;
        case AF_INET6:
            ipv6Addr = reinterpret_cast<struct sockaddr_in6 *>(addr->sockaddr);
            memset_s(ipv6Addr, sizeof(struct sockaddr_in6), 0, sizeof(struct sockaddr_in6));
            ipv6Addr->sin6_family = AF_INET6;
            ipv6Addr->sin6_port = 0;
            memcpy_s(&ipv6Addr->sin6_addr, sizeof(struct in6_addr), &in6addr_any, sizeof(struct in6_addr));
            addr->addrLen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
            break;
        default:
            LOG_ERROR(ERRNO_SOCK_NOT_SUPPORTED, "family unsupported: %u", family);
            return ERRNO_SOCK_NOT_SUPPORTED;
    }
    return 0;
}

#else
int SockRecvGeneral(int fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen, unsigned long long timeout)
{
    ssize_t recvRet;
    int ret;
    SchdpollEventType type = SHCDPOLL_READ;

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        do {
            recvRet = recv(fd, buf, static_cast<size_t>(len), flags);
            ret = errno;
        } while ((recvRet == -1) && (ret == EINTR || ret == 0));

        if (recvRet >= 0) {
            ret = 0;
            *recvLen = static_cast<int>(recvRet);
            break;
        }

        if (ret != EAGAIN) {
            LOG_ERROR(ret, "recv failed, fd: %d, len: %u", fd, len);
            break;
        }

        LOG_INFO(0, "recv waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWaitInlock(fd, type);
        } else {
            ret = SchdfdWaitInlockTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "recv wait over, fd: %d", fd);
        if (ret != 0) {
            break;
        }
    }
    SchdfdUnlock(fd, type);
    return ret;
}

int SockSendGeneral(int fd, const void *buf, unsigned int len, SocketFlag flags,
                    int *sendLen, unsigned long long timeout)
{
    ssize_t sendRet;
    int ret;
    SchdpollEventType type = SHCDPOLL_WRITE;

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        sendRet = send(fd, buf, (size_t)len, flags);
        if (sendRet > 0) {
            ret = 0;
            *sendLen = static_cast<int>(sendRet);
            break;
        }

        ret = errno;
        if (ret == 0) {
            continue;
        }
        if (ret != EAGAIN) {
            LOG_ERROR(ret, "send failed, fd: %d, len: %u", fd, len);
            break;
        }

        LOG_INFO(0, "send waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWaitInlock(fd, type);
        } else {
            ret = SchdfdWaitInlockTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "send wait over, fd: %d", fd);

        if (ret != 0) {
            break;
        }
    }
    SchdfdUnlock(fd, type);
    return ret;
}

int SockSendtoGeneral(int fd, void *bufAndLen, SocketFlag flags, const struct SockAddr *toAddr,
                      int *sendLen, unsigned long long timeout)
{
    ssize_t sendRet;
    int ret;
    SchdpollEventType type = SHCDPOLL_WRITE;
    struct SockBufInfo *bufInfo = static_cast<struct SockBufInfo *>(bufAndLen);

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        sendRet = sendto(fd, bufInfo->constBuf, (size_t)bufInfo->len,
                         flags, reinterpret_cast<struct sockaddr *>(toAddr->sockaddr), toAddr->addrLen);
        if (sendRet >= 0) {
            ret = 0;
            *sendLen = static_cast<int>(sendRet);
            break;
        }

        ret = errno;
        if (ret == 0) {
            continue;
        }
        if (ret != EAGAIN) {
            LOG_ERROR(ret, "sendto failed, fd: %d, len: %u", fd, bufInfo->len);
            break;
        }

        LOG_INFO(0, "sendto waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWaitInlock(fd, type);
        } else {
            ret = SchdfdWaitInlockTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "sendto wait over, fd: %d", fd);

        if (ret != 0) {
            break;
        }
    }

    SchdfdUnlock(fd, type);
    return ret;
}

int SockRecvfromGeneral(int fd, void *bufAndLen, SocketFlag flags, struct SockAddr *fromAddr,
                        int *recvLen, unsigned long long timeout)
{
    ssize_t recvRet;
    int ret;
    socklen_t addrLen = 0;
    SchdpollEventType type = SHCDPOLL_READ;
    struct SockBufInfo *bufInfo = static_cast<struct SockBufInfo *>(bufAndLen);
    struct sockaddr *sockAddr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ?
                                nullptr : reinterpret_cast<struct sockaddr *>(fromAddr->sockaddr);
    socklen_t *addrLenPtr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ? &addrLen : &(fromAddr->addrLen);
    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        do {
            recvRet = recvfrom(fd, bufInfo->buf, static_cast<size_t>(bufInfo->len),
                               flags, sockAddr, addrLenPtr);
            ret = errno;
        } while ((recvRet == -1) && (ret == EINTR || ret == 0));

        if (recvRet >= 0) {
            ret = 0;
            *recvLen = static_cast<int>(recvRet);
            break;
        }

        if (ret != EAGAIN) {
            LOG_ERROR(ret, "recvfrom failed, fd: %d, len: %u", fd, bufInfo->len);
            break;
        }

        LOG_INFO(0, "recvfrom waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWaitInlock(fd, type);
        } else {
            ret = SchdfdWaitInlockTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "recvfrom wait over, fd: %d", fd);

        if (ret != 0) {
            break;
        }
    }
    SchdfdUnlock(fd, type);
    return ret;
}

int SockSendNonBlockGeneral(int fd, const void *buf, unsigned int len, SocketFlag flags, int *sendLen)
{
    ssize_t sendRet;
    int ret;

    do {
        sendRet = send(fd, buf, (size_t)len, flags);
        if (sendRet > 0) {
            ret = 0;
            *sendLen = static_cast<int>(sendRet);
            return ret;
        }
        ret = errno;
    } while (ret == 0);

    if (ret != EAGAIN) {
        LOG_ERROR(ret, "send failed, fd: %d, len: %u", fd, len);
    }
    return ret;
}

int SockRecvNonBlockGeneral(int fd, void *buf, unsigned int len, SocketFlag flags, int *recvLen)
{
    ssize_t recvRet;
    int ret;
    int cycles = 0;
    bool scheduleFree = false;
    struct ScheduleProcessor *schdProcessor = &(ScheduleGet()->schdProcessor);
    unsigned int freeThreshold = schdProcessor->processorNum / PROCESSOR_FREE_RATIO;

    do {
        recvRet = recv(fd, buf, static_cast<size_t>(len), flags);
        ret = errno;
        if (recvRet != -1) {
            break;
        }
        cycles++;
        // If the number of idle processors is sufficient, spin recv some times to reduce codethread park.
        scheduleFree = (schdProcessor->freeNum).load(std::memory_order_relaxed) > freeThreshold &&
                        cycles <= RECV_MAX_CYCLES;
        (void)sched_yield();
    } while (ret == EINTR || ret == 0 || (ret == EAGAIN && scheduleFree));

    if (recvRet >= 0) {
        ret = 0;
        *recvLen = static_cast<int>(recvRet);
        return ret;
    }

    if (ret != EAGAIN) {
        LOG_ERROR(ret, "recv failed, fd: %d, len: %u", fd, len);
    }

    return ret;
}

int SockSendtoNonBlockGeneral(int fd, const void *buf, unsigned int len, SocketFlag flags,
                              const struct SockAddr *toAddr, int *sendLen)
{
    ssize_t sendRet;
    int ret;
    do {
        sendRet = sendto(fd, buf, static_cast<size_t>(len), flags,
                         reinterpret_cast<struct sockaddr *>(toAddr->sockaddr), toAddr->addrLen);
        if (sendRet >= 0) {
            ret = 0;
            *sendLen = static_cast<int>(sendRet);
            return ret;
        }
        ret = errno;
    } while (ret == 0);
    if (ret != EAGAIN) {
        LOG_ERROR(ret, "sendto failed, fd: %d, len: %u", fd, len);
    }
    return ret;
}

int SockRecvfromNonBlockGeneral(int fd, void *buf, unsigned int len, SocketFlag flags, struct SockAddr *fromAddr,
                                int *recvLen)
{
    ssize_t recvRet;
    int ret;
    socklen_t addrLen = 0;
    struct sockaddr *sockAddr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ?
                                nullptr : reinterpret_cast<struct sockaddr *>(fromAddr->sockaddr);
    socklen_t *addrLenPtr = (fromAddr == nullptr || fromAddr->sockaddr == nullptr) ? &addrLen : &(fromAddr->addrLen);
    do {
        recvRet = recvfrom(fd, buf, static_cast<size_t>(len),
                           flags, sockAddr, addrLenPtr);
        ret = errno;
    } while ((recvRet == -1) && (ret == EINTR || ret == 0));

    if (recvRet >= 0) {
        ret = 0;
        *recvLen = static_cast<int>(recvRet);
        return ret;
    }

    if (ret != EAGAIN) {
        LOG_ERROR(ret, "recvfrom failed, fd: %d, len: %u", fd, len);
    }
    return ret;
}

#ifdef MRT_LINUX

int SockCreateInternal(int domain, int type, int protocol, int *socketError)
{
    int sockFd = socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
    if (sockFd == -1) {
        *socketError = errno;
        LOG_ERROR(*socketError, "create socket failed, domain: %u, protocol: %d", domain, protocol);
        return -1;
    }
    return sockFd;
}

int SockAcceptInternal(int inFd, struct sockaddr *sockaddr, socklen_t *addrLen, int &globalErr, int &fcntlErr)
{
    (void)fcntlErr;
    int connFd = 0;
    do {
        connFd = accept4(inFd, sockaddr, addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    } while ((connFd == -1) && (errno == EINTR || errno == ECONNABORTED));
    globalErr = errno;
    return connFd;
}

#endif

#ifdef MRT_MACOS

int SockCreateInternal(int domain, int type, int protocol, int *socketError)
{
    int sockFd = socket(domain, type, protocol);
    if (sockFd == -1) {
        *socketError = errno;
        LOG_ERROR(*socketError, "create socket failed, domain: %u, protocol: %d", domain, protocol);
        return -1;
    }
    int floexec = fcntl(sockFd, F_SETFD, fcntl(sockFd, F_GETFD, 0) | FD_CLOEXEC);
    int nonblock = fcntl(sockFd, F_SETFL, fcntl(sockFd, F_GETFL, 0) | O_NONBLOCK);
    if (floexec == -1 || nonblock == -1) {
        *socketError = errno;
        LOG_ERROR(*socketError, "set attribute failed when create socket, sockFd: %d", sockFd);
        return -1;
    }
    return sockFd;
}

int SockAcceptInternal(int inFd, struct sockaddr *sockaddr, socklen_t *addrLen, int &globalErr, int &fcntlErr)
{
    int connFd = 0;
    int acceptErr = 0;
    do {
        connFd = accept(inFd, sockaddr, addrLen);
        acceptErr = errno;
        int floexec = fcntl(connFd, F_SETFD, fcntl(connFd, F_GETFD, 0) | FD_CLOEXEC);
        int nonblock = fcntl(connFd, F_SETFL, fcntl(connFd, F_GETFL, 0) | O_NONBLOCK);
        if (connFd != -1 && (floexec == -1 || nonblock == -1)) {
            LOG_ERROR(errno, "accept failed when fcntl fd: %d", connFd);
            fcntlErr = errno;
            return -1;
        }
    } while ((connFd == -1) && (acceptErr == EINTR || acceptErr == ECONNABORTED));
    globalErr = acceptErr;
    return connFd;
}

#endif
#endif

#ifdef __cplusplus
}
#endif
