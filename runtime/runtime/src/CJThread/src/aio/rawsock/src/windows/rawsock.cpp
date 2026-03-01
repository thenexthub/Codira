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
#include "rawsock.h"

#ifdef __cplusplus
extern "C" {
#endif

int RawsockIsConnectionOriented(SOCKET fd, bool *isConnectionOriented)
{
    int error;
    int sockType;
    int optlen = sizeof(int);
    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, reinterpret_cast<char *>(&sockType), &optlen) == SOCKET_ERROR) {
        error = WSAGetLastError();
        LOG_ERROR(error, "getsockopt failed, fd: %d, level: %d, optname: %d", fd, SOL_SOCKET, SO_TYPE);
        return error;
    }
    *isConnectionOriented = (sockType == SOCK_STREAM || sockType == SOCK_RDM || sockType == SOCK_SEQPACKET) ?
                             true :false;
    return 0;
}

SOCKET RawsockCreate(int domain, int type, int protocol, int *socketError)
{
    SOCKET sockFd;
    int error;

    // UDS is not supported in Windows.
    if (domain == AF_UNIX) {
        *socketError = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "domain: %d out of compliance", domain);
        return -1;
    }
    sockFd = WSASocket(domain, type, protocol, nullptr,
                       0, WSA_FLAG_OVERLAPPED);
    if (sockFd == INVALID_SOCKET) {
        *socketError = WSAGetLastError();
        LOG_ERROR(*socketError, "WSASocket failed, domain: %u, type %d, protocol: %d", domain, type, protocol);
        return -1;
    }
    error = SchdfdRegisterAndNetpollAdd(sockFd);
    if (error != 0) {
        *socketError = error;
        LOG_ERROR(error, "rawsock SchdfdRegisterAndNetpollAdd failed, fd: %d", sockFd);
        return -1;
    }
    return sockFd;
}

int RawsockBind(SOCKET fd, const struct SockAddr *addr, int backlog)
{
    int error;
    struct sockaddr *unixSockAddr = reinterpret_cast<struct sockaddr *>(addr->sockaddr);
    socklen_t addrLen = addr->addrLen;
    (void)backlog;

    if (bind(fd, unixSockAddr, addrLen) == SOCKET_ERROR) {
        error = WSAGetLastError();
        LOG_ERROR(error, "bind failed, fd: %d, addrlen: %u", fd, addrLen);
        return error;
    }

    return 0;
}

int RawsockListen(SOCKET sockFd, int backlog)
{
    int error;

    // SOMAXCONN_HINT(N) Indicates that backlog is set to N.
    // The value range is (200, 65535).
    if (listen(sockFd, SOMAXCONN_HINT(backlog)) == SOCKET_ERROR) {
        error = WSAGetLastError();
        LOG_ERROR(error, "listen failed, fd: %d", sockFd);
        return error;
    }
    LOG_INFO(0, "listen success, fd: %d", sockFd);
    return 0;
}

int RawsockAcceptCoreInlock(SOCKET listenFd, struct sockaddr_storage *localAndPeerAddr,
                            DWORD addrLen, SOCKET *outFd, unsigned long long timeout)
{
    int error;
    int acceptRet;
    SOCKET acceptFd;
    struct IocpOperation *operation;
    SchdpollEventType type = SHCDPOLL_READ;
    WSAPROTOCOL_INFO wsaprotocolInfo;
    int optlen = sizeof(wsaprotocolInfo);

    operation = SchdfdUpdateIocpOperationInlock(listenFd, type, nullptr, 0);
    if (operation == nullptr) {
        error = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(error, "get iocp read operation failed , fd: %llu", listenFd);
        return error;
    }
    if (getsockopt(listenFd, SOL_SOCKET, SO_PROTOCOL_INFO,
                   reinterpret_cast<char *>(&wsaprotocolInfo), &optlen) == SOCKET_ERROR) {
        error = WSAGetLastError();
        LOG_ERROR(error, "getsockopt failed, fd: %d, level: %d, optname: %d", listenFd, SOL_SOCKET, SO_PROTOCOL_INFO);
        return error;
    }
    // Create a socket that supports overlapping I/O operations.
    acceptFd = WSASocket(wsaprotocolInfo.iAddressFamily, wsaprotocolInfo.iSocketType, wsaprotocolInfo.iProtocol,
                         nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (acceptFd == INVALID_SOCKET) {
        error = WSAGetLastError();
        LOG_ERROR(error, "WSASocket failed, domain: %u, protocol: %d", wsaprotocolInfo.iAddressFamily,
                  wsaprotocolInfo.iProtocol);
        return error;
    }

    // After the asynchronous accept operation is delivered, a new connection is accepted.
    // The asynchronous interface returns a response immediately.
    // Returns the local and remote addresses of the connection fd.
    acceptRet = g_mswsockHooks.AcceptEx(listenFd, acceptFd, localAndPeerAddr, 0,
                                        addrLen, addrLen, &operation->transBytes, &operation->overlapped);
    if (acceptRet == TRUE && SchdfdUseSkipIocp()) {
        *outFd = acceptFd;
        return 0;
    }

    if (acceptRet == FALSE) {
        error = WSAGetLastError();
        if (error != ERROR_IO_PENDING) {
            LOG_ERROR(error, "AcceptEx failed, fd: %llu", listenFd);
            (void)closesocket(acceptFd);
            return error;
        }
    }

    error = SchdfdIocpWaitInlock(operation, timeout);
    if (error != 0) {
        (void)closesocket(acceptFd);
        return error;
    }

    *outFd = acceptFd;
    return 0;
}

int RawsockAccept(SOCKET inFd, SOCKET *outFd, struct SockAddr *addr, unsigned long long timeout)
{
    int error;
    SOCKET fd;
    struct sockaddr_storage localAndPeerAddr[2];
    struct sockaddr *localAddr;
    struct sockaddr *peerAddr;
    int localAddrLen;
    int peerAddrLen;
    DWORD addrLen = sizeof(struct sockaddr_storage);
    SchdpollEventType type = SHCDPOLL_READ;

    error = SchdfdLock(inFd, type);
    if (error != 0) {
        return error;
    }

    while (1) {
        error = RawsockAcceptCoreInlock(inFd, localAndPeerAddr, addrLen, &fd, timeout);
        if (error != 0) {
            if (error == WSAECONNRESET || error == ERROR_NETNAME_DELETED) {
                continue;
            }
            SchdfdUnlock(inFd, type);
            return error;
        } else {
            break;
        }
    }

    // Use getsockname and getpeername interfaces on this socket to get local and remote addresses.
    error = setsockopt(fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char *>(&inFd), sizeof(inFd));
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        LOG_ERROR(error, "setsockopt failed, fd: %llu", fd);
        SchdfdUnlock(inFd, type);
        (void)closesocket(fd);
        return error;
    }

    SchdfdUnlock(inFd, type);

    if (addr != nullptr && addr->sockaddr != nullptr) {
        // AcceptEx puts the application data, local address, and remote address into the same buffer.
        // This function parses the three parts of data.
        g_mswsockHooks.GetAcceptExSockaddrs(localAndPeerAddr, 0, addrLen, addrLen,
                                            &localAddr, &localAddrLen, &peerAddr, &peerAddrLen);
        (void)memcpy_s(addr->sockaddr, peerAddrLen, peerAddr, peerAddrLen);
    }

    error = SchdfdRegisterAndNetpollAdd(fd);
    if (error != 0) {
        LOG_ERROR(error, "rawsock SchdfdRegisterAndNetpollAdd failed, fd: %d", fd);
        return error;
    }
    *outFd = fd;
    return 0;
}

int RawsockConnectOriented(SOCKET fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout)
{
    int error;
    int connectRet;
    SchdpollEventType type = SHCDPOLL_WRITE;
    struct IocpOperation *operation;

    error = SchdfdLock(fd, type);
    if (error != 0) {
        return error;
    }

    operation = SchdfdUpdateIocpOperationInlock(fd, type, nullptr, 0);
    if (operation == nullptr) {
        error = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(error, "get iocp write operation failed , fd: %llu", fd);
        SchdfdUnlock(fd, type);
        return error;
    }

    // After the asynchronous connection operation is delivered,
    // the asynchronous interface returns a message immediately.
    // The ConnectEx function requires fd to call bind beforehand and only supports connection-oriented sockets.
    connectRet = g_mswsockHooks.ConnectEx(fd, addr, addrLen, nullptr, 0, nullptr, &operation->overlapped);
    if (connectRet == TRUE && SchdfdUseSkipIocp()) {
        SchdfdUnlock(fd, type);
        return 0;
    }

    if (connectRet == FALSE) {
        error = WSAGetLastError();
        if (error != ERROR_IO_PENDING) {
            LOG_ERROR(error, "connect failed, fd: %llu", fd);
            SchdfdUnlock(fd, type);
            return error;
        }
    }

    error = SchdfdIocpWaitInlock(operation, timeout);
    if (error != 0) {
        SchdfdUnlock(fd, type);
        return error;
    }

    SchdfdUnlock(fd, type);
    return 0;
}

int RawsockBindConnect(SOCKET connFd, struct SockAddr *local,
                       struct SockAddr *peer, unsigned long long timeout)
{
    int error;
    bool isConnectionOriented;
    error = RawsockIsConnectionOriented(connFd, &isConnectionOriented);
    if (error != 0) {
        return error;
    }
    if (local != nullptr && local->sockaddr != nullptr) {
        error = RawsockBind(connFd, local, 0);
        if (error != 0) {
            return error;
        }
    }

    if (isConnectionOriented) {
        error = RawsockConnectOriented(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr),
                                       peer->addrLen, timeout);
        if (error != 0) {
            return error;
        }
        // Use getsockname and getpeername interfaces on this socket to get local and remote addresses.
        error = setsockopt(connFd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,
                           reinterpret_cast<char *>(&connFd), sizeof(connFd));
        if (error == SOCKET_ERROR) {
            error = WSAGetLastError();
            LOG_ERROR(error, "setsockopt failed, fd: %llu", connFd);
            return error;
        }
    } else {
        error = connect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen);
        if (error != 0) {
            error = WSAGetLastError();
            LOG_ERROR(error, "connect failed, fd: %d", connFd);
            return error;
        }
    }

    return 0;
}

int RawsockDisconnect(SignedSocket connFd)
{
    struct sockaddr_storage peerAddr;
    socklen_t peerLen = sizeof(struct sockaddr_storage);
    int error;
    bool isConnectionOriented;
    error = RawsockIsConnectionOriented(connFd, &isConnectionOriented);
    if (error != 0) {
        return error;
    }
    if (isConnectionOriented) {
        LOG_ERROR(ERRNO_SOCK_NOT_SUPPORTED, "ConnectionOriented socket unsupported disconnect!");
        return ERRNO_SOCK_NOT_SUPPORTED;
    }
    memset_s(&peerAddr, sizeof(struct sockaddr_storage), 0, sizeof(struct sockaddr_storage));
    peerAddr.ss_family = AF_UNSPEC;
    error = connect(connFd, reinterpret_cast<const struct sockaddr *>(&peerAddr), peerLen);
    if (error != 0) {
        error = WSAGetLastError();
        LOG_ERROR(error, "connect failed, fd: %d", connFd);
        return error;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
