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


#include "sock_impl.h"
#include "udpsock.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (MRT_LINUX) || defined (MRT_MACOS)

int UdpsockCreate(int domain, int type, int protocol, int *socketError)
{
    int fd;
    int ret = 0;
    if (type != SOCK_DGRAM) {
        *socketError = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "udp socket type invalid: %d", type);
        return -1;
    }
    // create socket and set NONBLOCK, SOCK_CLOEXEC
    fd = SockCreateInternal(domain, type, protocol, socketError);
    if (fd == -1) {
        return -1;
    }
    ret = SchdfdRegisterAndNetpollAdd(fd);
    if (ret != 0) {
        *socketError = ret;
        LOG_ERROR(ret, "udpsock SchdfdRegisterAndNetpollAdd failed, fd: %d", fd);
        return -1;
    }

    LOG_INFO(0, "udp socket create success, fd: %d", fd);
    return fd;
}

#endif

int UdpsockBind(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen)
{
    int err;
    unsigned short port = 0;
    char ipBuf[INET6_ADDRSTRLEN] = {0};

    if (bind(fd, addr, addrLen) == -1) {
#ifdef MRT_WINDOWS
        err = WSAGetLastError();
#else
        err = errno;
#endif
        if (addr->sa_family == AF_INET) {
            (void)inet_ntop(AF_INET, &(reinterpret_cast<const struct sockaddr_in *>(addr))->sin_addr,
                            ipBuf, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in *>(addr))->sin_port);
        } else if (addr->sa_family == AF_INET6) {
            (void)inet_ntop(AF_INET6, &(reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_addr,
                            ipBuf, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_port);
        }

        LOG_ERROR(err, "udpsock bind failed, fd: %d, ip: %s, port: %u, addrlen: %u", fd, ipBuf, port, addrLen);
        return err;
    }
    LOG_INFO(0, "udpsock bind success, fd: %d", fd);
    return 0;
}

int UdpsockBindNoListen(SignedSocket sockFd, const struct SockAddr *addr, int backlog)
{
    int ret;
    (void)backlog;

    ret = UdpsockBind(sockFd, reinterpret_cast<const struct sockaddr *>(addr->sockaddr), addr->addrLen);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int UdpsockConnect(SignedSocket fd, const struct sockaddr *addr, socklen_t addrLen)
{
    int ret;

    ret = connect(fd, addr, addrLen);
    if (ret != 0) {
#ifdef MRT_WINDOWS
        ret = WSAGetLastError();
#else
        ret = errno;
#endif
        LOG_ERROR(ret, "udp connect failed, fd: %d", fd);
        return ret;
    }
    LOG_INFO(0, "connect success, fd: %d", fd);

    return 0;
}

int UdpsockBindConnect(SignedSocket connFd, struct SockAddr *local, struct SockAddr *peer, unsigned long long timeout)
{
    int ret;
    (void)timeout;

    if (local != nullptr && local->sockaddr != nullptr) {
        ret = UdpsockBind(connFd, reinterpret_cast<const struct sockaddr *>(local->sockaddr), local->addrLen);
        if (ret != 0) {
            LOG_ERROR(ret, "UdpsockBind failed");
            return ret;
        }
    }

    ret = UdpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen);
    if (ret != 0) {
        LOG_ERROR(ret, "UdpsockConnect failed");
        return ret;
    }

    return 0;
}

/* register socket hooks */
void UdpsockRegisterSocketHooks(struct SockCommHooks *hooks)
{
    hooks->bind = UdpsockBindNoListen;
    hooks->listen = nullptr;
    hooks->accept = nullptr;
    hooks->connect = UdpsockBindConnect;
    hooks->disconnect = UdpsockDisconnect;
#ifdef MRT_MACOS
    hooks->disconnectForIPv6 = UdpsockDisconnectForIPv6;
#endif
    hooks->send = SockSendGeneral;
    hooks->sendNonBlock = nullptr;
    hooks->waitSend = nullptr;
    hooks->recv = SockRecvGeneral;
    hooks->recvNonBlock = nullptr;
    hooks->waitRecv = nullptr;
    hooks->close = SockCloseGeneral;
    hooks->shutdown = SockShutdownGeneral;
    hooks->keepAliveSet = nullptr;
    hooks->sockAddrGet = SockAddrGetGeneral;
    hooks->recvfrom = SockRecvfromGeneral;
    hooks->recvfromNonBlock = nullptr;
    hooks->sendto = SockSendtoGeneral;
    hooks->sendtoNonBlock = nullptr;
    hooks->createSocket = UdpsockCreate;
    hooks->optionSet = SockOptionSetGeneral;
    hooks->optionGet = SockOptionGetGeneral;
}

__attribute__((constructor)) int UdpsockInit(void)
{
    int ret;
    struct SockCommHooks hooks;

    UdpsockRegisterSocketHooks(&hooks);
    ret = SockCommHooksReg(NET_TYPE_UDP, &hooks);

    return ret;
}

#ifdef __cplusplus
}
#endif
