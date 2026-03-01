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


#include "log.h"
#include "sock_impl.h"
#include "schdfd_impl.h"
#include "securec.h"
#include "tcpsock.h"

#ifdef __cplusplus
extern "C" {
#endif

SOCKET TcpsockCreate(int domain, int type, int protocol, int *socketError)
{
    SOCKET sockFd;
    int ret;
    (void)type;

    sockFd = WSASocket(domain, SOCK_STREAM, protocol, nullptr,
                       0, WSA_FLAG_OVERLAPPED);
    if (sockFd == INVALID_SOCKET) {
        *socketError = WSAGetLastError();
        LOG_ERROR(*socketError, "WSASocket failed, domain: %u, protocol: %d", domain, protocol);
        return -1;
    }
    ret = SchdfdRegisterAndNetpollAdd(sockFd);
    if (ret != 0) {
        *socketError = ret;
        LOG_ERROR(ret, "tcpsock SchdfdRegisterAndNetpollAdd failed, fd: %d", sockFd);
        return -1;
    }
    return sockFd;
}

int TcpsockBind(SOCKET fd, const struct sockaddr *addr, socklen_t addrLen)
{
    int ret;
    char dst[INET6_ADDRSTRLEN] = {0};
    unsigned short port = 0;

    if (bind(fd, addr, addrLen) == SOCKET_ERROR) {
        ret = WSAGetLastError();
        if (addr->sa_family == AF_INET) {
            (void)inet_ntop(AF_INET, &(reinterpret_cast<const struct sockaddr_in *>(addr))->sin_addr,
                            dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in *>(addr))->sin_port);
        } else if (addr->sa_family == AF_INET6) {
            (void)inet_ntop(AF_INET6, &(reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_addr,
                            dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_port);
        }

        LOG_ERROR(ret, "bind failed, fd: %llu, ip: %s, port: %u, addrlen: %u", fd, dst, port, addrLen);
        return ret;
    }

    return 0;
}

int TcpsockBindListen(SOCKET sockFd, const struct SockAddr *addr, int backlog)
{
    int ret = TcpsockBind(sockFd, reinterpret_cast<const struct sockaddr *>(addr->sockaddr), addr->addrLen);
    if (ret != 0) {
        return ret;
    }

    // SOMAXCONN_HINT(N) means to set the backlog to N, with a setting range of (200, 65535).
    if (listen(sockFd, SOMAXCONN_HINT(backlog)) == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "listen failed, fd: %d", sockFd);
        return ret;
    }

    return 0;
}

int TcpsockAcceptCoreInlock(SOCKET listenFd, struct sockaddr_storage *localAndPeerAddr,
                            DWORD addrLen, SOCKET *outFd, unsigned long long timeout)
{
    int ret;
    int acceptRet;
    SOCKET acceptFd;
    struct IocpOperation *operation;
    SchdpollEventType type = SHCDPOLL_READ;

    operation = SchdfdUpdateIocpOperationInlock(listenFd, type, nullptr, 0);
    if (operation == nullptr) {
        ret = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ret, "get iocp read operation failed , fd: %llu", listenFd);
        return ret;
    }

    // Create sockets that support overlapping I/O operations.
    acceptFd = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr,
                         0, WSA_FLAG_OVERLAPPED);
    if (acceptFd == INVALID_SOCKET) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "WSASocket failed, domain: %u, protocol: %d", AF_INET, 0);
        return ret;
    }

    // Submit asynchronous accept operation, accept new connection, and the asynchronous
    // interface returns immediately. Return connection fd, local and remote addresses.
    acceptRet = g_mswsockHooks.AcceptEx(listenFd, acceptFd, localAndPeerAddr, 0,
                                        addrLen, addrLen, &operation->transBytes, &operation->overlapped);
    if (acceptRet == TRUE && SchdfdUseSkipIocp()) {
        *outFd = acceptFd;
        return 0;
    }

    if (acceptRet == FALSE) {
        ret = WSAGetLastError();
        if (ret != ERROR_IO_PENDING) {
            LOG_ERROR(ret, "AcceptEx failed, fd: %llu", listenFd);
            (void)closesocket(acceptFd);
            return ret;
        }
    }

    ret = SchdfdIocpWaitInlock(operation, timeout);
    if (ret != 0) {
        (void)closesocket(acceptFd);
        return ret;
    }

    *outFd = acceptFd;
    return 0;
}

int TcpsockAccept(SOCKET inFd, SOCKET *outFd, struct SockAddr *addr, unsigned long long timeout)
{
    int ret;
    SOCKET fd;
    struct sockaddr_storage localAndPeerAddr[2];
    struct sockaddr *peerAddr;
    struct sockaddr *localAddr;
    int peerAddrLen;
    int localAddrLen;
    DWORD addrLen = sizeof(struct sockaddr_storage);
    SchdpollEventType type = SHCDPOLL_READ;

    ret = SchdfdLock(inFd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        ret = TcpsockAcceptCoreInlock(inFd, localAndPeerAddr, addrLen, &fd, timeout);
        if (ret != 0) {
            if (ret == WSAECONNRESET || ret == ERROR_NETNAME_DELETED) {
                continue;
            }
            SchdfdUnlock(inFd, type);
            return ret;
        } else {
            break;
        }
    }

    // After setting this option, you can use the getSockname and getPeername
    // interfaces on this socket to obtain local and remote addresses.
    ret = setsockopt(fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char *>(&inFd), sizeof(inFd));
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "setsockopt failed, fd: %llu", fd);
        SchdfdUnlock(inFd, type);
        (void)closesocket(fd);
        return ret;
    }

    SchdfdUnlock(inFd, type);

    if (addr != nullptr && addr->sockaddr != nullptr) {
        // AcceptEx puts application data, local address, and remote address into the same buffer,
        // and this function parses these three parts of data.
        g_mswsockHooks.GetAcceptExSockaddrs(localAndPeerAddr, 0, addrLen, addrLen,
                                            &localAddr, &localAddrLen, &peerAddr, &peerAddrLen);
        (void)memcpy_s(addr->sockaddr, peerAddrLen, peerAddr, peerAddrLen);
    }

    ret = SchdfdRegisterAndNetpollAdd(fd);
    if (ret != 0) {
        LOG_ERROR(ret, "tcpsock SchdfdRegisterAndNetpollAdd failed, fd: %d", fd);
        return ret;
    }
    *outFd = fd;
    return 0;
}

int TcpsockConnect(SOCKET fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout)
{
    int ret;
    int connectRet;
    SchdpollEventType type = SHCDPOLL_WRITE;
    struct IocpOperation *operation;

    ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }

    operation = SchdfdUpdateIocpOperationInlock(fd, type, nullptr, 0);
    if (operation == nullptr) {
        ret = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ret, "get iocp write operation failed , fd: %llu", fd);
        SchdfdUnlock(fd, type);
        return ret;
    }

    // Submit asynchronous connection operation, and the asynchronous interface will immediately return.
    // The ConnectEx function requires fd to call bind beforehand and only supports connection oriented sockets.
    connectRet = g_mswsockHooks.ConnectEx(fd, addr, addrLen, nullptr, 0, nullptr, &operation->overlapped);
    if (connectRet == TRUE && SchdfdUseSkipIocp()) {
        SchdfdUnlock(fd, type);
        return 0;
    }

    if (connectRet == FALSE) {
        ret = WSAGetLastError();
        if (ret != ERROR_IO_PENDING) {
            LOG_ERROR(ret, "connect failed, fd: %llu", fd);
            SchdfdUnlock(fd, type);
            return ret;
        }
    }

    ret = SchdfdIocpWaitInlock(operation, timeout);
    if (ret != 0) {
        SchdfdUnlock(fd, type);
        return ret;
    }

    SchdfdUnlock(fd, type);
    return 0;
}

/* The ConnectEx function requires fd to call bind beforehand. */
int ConnectBindLocal(SOCKET connFd, const struct SockAddr *local, short family)
{
    int ret;
    struct SockAddr localDefauleAddr;
    const struct SockAddr *pAddr = local;
    struct sockaddr_storage localAddr;
    localDefauleAddr.sockaddr = &localAddr;
    if (pAddr == nullptr || pAddr->sockaddr == nullptr) {
        ret = SockGetLocalDefaultAddr(family, &localDefauleAddr);
        if (ret != 0) {
            return ret;
        }
        pAddr = &localDefauleAddr;
    }

    ret = TcpsockBind(connFd, reinterpret_cast<const struct sockaddr *>(pAddr->sockaddr), pAddr->addrLen);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int TcpsockBindConnect(SOCKET connFd, struct SockAddr *local,
                       struct SockAddr *peer, unsigned long long timeout)
{
    int ret;

    ret = ConnectBindLocal(connFd, local, peer->sockaddr->ss_family);
    if (ret != 0) {
        return ret;
    }

    ret = TcpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen, timeout);
    if (ret != 0) {
        return ret;
    }

    // After setting this option, you can use the getSockname and getPeername interfaces
    // on this socket to obtain local and remote addresses.
    ret = setsockopt(connFd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, reinterpret_cast<char *>(&connFd), sizeof(connFd));
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "setsockopt failed, fd: %llu", connFd);
        return ret;
    }

    return 0;
}

int TcpsockKeepAliveSet(SOCKET fd, const struct SockKeepAliveCfg *cfg)
{
    int ret;

    ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&cfg->keepAlive, sizeof(BOOL));
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        LOG_ERROR(ret, "setsockopt failed, fd: %d, val: %u", fd, cfg->keepAlive);
        return ret;
    }

    if (cfg->count != 0 || cfg->idle != 0 || cfg->interval != 0) {
        LOG_WARN(0, "count,idle and interval don't support yet");
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
