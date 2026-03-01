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
#include "schdfd_impl.h"
#include "tcpsock.h"
#if defined (MRT_LINUX) || defined (MRT_MACOS)
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include "log.h"
#include "sock_impl.h"
#include "securec.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (MRT_LINUX) || defined (MRT_MACOS)

int TcpsockCreate(int domain, int type, int protocol, int *socketError)
{
    int sockFd;
    int ret = 0;
    if (type != SOCK_STREAM) {
        *socketError = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "socket type invalid: %d", type);
        return -1;
    }
    // create socket and set NONBLOCK, SOCK_CLOEXEC
    sockFd = SockCreateInternal(domain, type, protocol, socketError);
    if (sockFd == -1) {
        return -1;
    }
    ret = SchdfdRegister(sockFd);
    if (ret != 0) {
        (void)close(sockFd);
        *socketError = ret;
        LOG_ERROR(ret, "sockFd SchdfdRegister failed!");
        return -1;
    }
    LOG_INFO(0, "socket create success, fd: %d", sockFd);
    return sockFd;
}

int TcpsockBind(int fd, const struct sockaddr *addr, socklen_t addrLen)
{
    int ret;
    char dst[INET6_ADDRSTRLEN] = {0};
    unsigned short port = 0;

    if (bind(fd, addr, addrLen) == -1) {
        ret = errno;
        if (addr->sa_family == AF_INET) {
            (void)inet_ntop(AF_INET, &(reinterpret_cast<const struct sockaddr_in *>(addr))->sin_addr,
                            dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in *>(addr))->sin_port);
        } else if (addr->sa_family == AF_INET6) {
            (void)inet_ntop(AF_INET6, &(reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_addr,
                            dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<const struct sockaddr_in6 *>(addr))->sin6_port);
        }

        LOG_ERROR(ret, "bind failed, fd: %d, ip: %s, port: %u, addrlen: %u", fd, dst, port, addrLen);
        return ret;
    }
    LOG_INFO(0, "bind success, fd: %d", fd);
    return 0;
}

int TcpsockBindListen(int sockFd, const struct SockAddr *addr, int backlog)
{
    SchdpollEventType type = SHCDPOLL_READ;
    int ret = SchdfdLock(sockFd, type);
    if (ret != 0) {
        return ret;
    }
    ret = TcpsockBind(sockFd, reinterpret_cast<const struct sockaddr *>(addr->sockaddr), addr->addrLen);
    if (ret != 0) {
        SchdfdUnlock(sockFd, type);
        return ret;
    }

    if (listen(sockFd, backlog) == -1) {
        ret = errno;
        LOG_ERROR(ret, "listen failed, fd: %d", sockFd);
        SchdfdUnlock(sockFd, type);
        return ret;
    }
    LOG_INFO(0, "listen success, fd: %d", sockFd);
    ret = SchdfdNetpollAdd(sockFd);
    if (ret != 0) {
        LOG_ERROR(ret, "sockFd SchdfdNetpollAdd failed!");
        SchdfdUnlock(sockFd, type);
        return ret;
    }
    SchdfdUnlock(sockFd, type);
    return 0;
}

int TcpsockAccept(int inFd, int *outFd, struct SockAddr *addr, unsigned long long timeout)
{
    int fd;
    int fcntlErr = 0;
    SchdpollEventType type = SHCDPOLL_READ;
    struct SockAddr sockaddr;
    struct SockAddr *sockaddrPtr = addr;
    struct sockaddr_storage sockaddrStorage;
    if (sockaddrPtr == nullptr || sockaddrPtr->sockaddr == nullptr) {
        sockaddrPtr = &sockaddr;
        sockaddr.sockaddr = &sockaddrStorage;
        sockaddr.addrLen = sizeof(struct sockaddr_storage);
    }
    int ret = SchdfdLock(inFd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        fd = SockAcceptInternal(inFd, reinterpret_cast<struct sockaddr *>(sockaddrPtr->sockaddr),
                                &sockaddrPtr->addrLen, ret, fcntlErr);
        if (fcntlErr != 0) {
            SchdfdUnlock(inFd, type);
            return fcntlErr;
        }
        if (fd >= 0) {
            if (LogInfoWritable()) {
                TcpsockAcceptSuccessLogWrite(reinterpret_cast<struct sockaddr *>(sockaddrPtr->sockaddr), inFd, fd);
            }
            break;
        }

        if (ret != EAGAIN) {
            SchdfdUnlock(inFd, type);
            LOG_ERROR(ret, "accept failed, fd: %d", inFd);
            return ret;
        }

        LOG_INFO(0, "accept waiting, fd: %d", inFd);
        ret = (timeout == static_cast<unsigned long long>(-1)) ? SchdfdWaitInlock(inFd, type) :
              SchdfdWaitInlockTimeout(inFd, type, timeout);
        LOG_INFO(0, "accept wait over, fd: %d", inFd);

        if (ret != 0) {
            SchdfdUnlock(inFd, type);
            return ret;
        }
    }
    SchdfdUnlock(inFd, type);

    ret = SchdfdRegisterAndNetpollAdd(fd);
    if (ret != 0) {
        LOG_ERROR(ret, "tcpsock SchdfdRegisterAndNetpollAdd failed, fd: %d", fd);
        return ret;
    }
    *outFd = fd;
    return 0;
}

void TcpsockAcceptSuccessLogWrite(struct sockaddr *addr, int inFd, int fd)
{
    char dst[INET6_ADDRSTRLEN] = {0};
    unsigned short port = 0;
    if (addr->sa_family == AF_INET) {
        (void)inet_ntop(AF_INET, &(reinterpret_cast<struct sockaddr_in *>(addr))->sin_addr,
                        dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<struct sockaddr_in *>(addr))->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        (void)inet_ntop(AF_INET6, &(reinterpret_cast<struct sockaddr_in6 *>(addr))->sin6_addr,
                        dst, INET6_ADDRSTRLEN);
            port = ntohs((reinterpret_cast<struct sockaddr_in6 *>(addr))->sin6_port);
    }
    LOG_INFO(0, "accept success, inFd: %d. accept fd: %d, ip: %s, port: %u",
             inFd, fd, dst, port);
}

int TcpsockConnect(int fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout)
{
    int ret;
    int opt;
    socklen_t optLen = static_cast<socklen_t>(sizeof(int));
    SchdpollEventType type = SHCDPOLL_WRITE;
    ret = connect(fd, addr, addrLen);
    if (ret == 0) {
        return SchdfdNetpollAdd(fd);
    }
    // The connection is in progress and needs to wait for a writable event to  complete the connection.
    // getsockopt will retrieve the connection result.
    if (errno == EINPROGRESS) {
        ret = SchdfdNetpollAdd(fd);
        if (ret != 0) {
            LOG_ERROR(ret, "sockFd SchdfdNetpollAdd failed!");
            return ret;
        }
        LOG_INFO(0, "connect waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWait(fd, type);
        } else {
            ret = SchdfdWaitTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "connect wait over, fd: %d", fd);

        if (ret != 0) {
            return ret;
        }

        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &optLen) == -1) {
            ret = errno;
            LOG_ERROR(ret, "getsockopt failed, fd: %d", fd);
            return ret;
        }

        if (opt != 0) {
            LOG_ERROR(opt, "connect failed, fd: %d", fd);
            return opt;
        }
        return 0;
    }

    ret = errno;
    LOG_ERROR(ret, "connect failed, fd: %d", fd);
    return ret;
}

int TcpsockBindConnect(int connFd, struct SockAddr *local, struct SockAddr *peer, unsigned long long timeout)
{
    SchdpollEventType type = SHCDPOLL_WRITE;
    int ret;

    ret = SchdfdLock(connFd, type);
    if (ret != 0) {
        return ret;
    }

    if (local != nullptr && local->sockaddr != nullptr) {
        ret = TcpsockBind(connFd, reinterpret_cast<const struct sockaddr *>(local->sockaddr), local->addrLen);
        if (ret != 0) {
            LOG_ERROR(ret, "TcpsockBind failed");
            SchdfdUnlock(connFd, type);
            return ret;
        }
    }

    ret = TcpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen, timeout);
    if (ret != 0) {
        LOG_ERROR(ret, "TcpsockConnect failed");
        SchdfdUnlock(connFd, type);
        return ret;
    }
    SchdfdUnlock(connFd, type);
    return 0;
}

int TcpsockWait(int fd, SchdpollEventType type, unsigned long long timeout)
{
    int ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }
    LOG_INFO(0, "waiting, fd: %d", fd);
    if (timeout == static_cast<unsigned long long>(-1)) {
        ret = SchdfdWaitInlock(fd, type);
    } else {
        ret = SchdfdWaitInlockTimeout(fd, type, timeout);
    }
    LOG_INFO(0, "wait over, fd: %d", fd);
    SchdfdUnlock(fd, type);
    return ret;
}

int TcpsockWaitSend(SignedSocket fd, unsigned long long timeout)
{
    return TcpsockWait(fd, SHCDPOLL_WRITE, timeout);
}

int TcpsockWaitRecv(SignedSocket fd, unsigned long long timeout)
{
    return TcpsockWait(fd, SHCDPOLL_READ, timeout);
}

#endif

/* register tcp socket hooks. */
void TcpsockRegisterSocketHooks(struct SockCommHooks *hooks)
{
    hooks->bind = TcpsockBindListen;
    hooks->listen = nullptr;
    hooks->accept = TcpsockAccept;
    hooks->connect = TcpsockBindConnect;
    hooks->disconnect = nullptr;
    hooks->send = SockSendGeneral;
    hooks->recv = SockRecvGeneral;
#ifdef MRT_WINDOWS
    hooks->sendNonBlock = nullptr;
    hooks->waitSend = nullptr;
    hooks->recvNonBlock = nullptr;
    hooks->waitRecv = nullptr;
#else
    hooks->sendNonBlock = SockSendNonBlockGeneral;
    hooks->waitSend = TcpsockWaitSend;
    hooks->recvNonBlock = SockRecvNonBlockGeneral;
    hooks->waitRecv = TcpsockWaitRecv;
#endif
    hooks->close = SockCloseGeneral;
    hooks->shutdown = SockShutdownGeneral;
    hooks->keepAliveSet = TcpsockKeepAliveSet;
    hooks->sockAddrGet = SockAddrGetGeneral;
    hooks->recvfrom = nullptr;
    hooks->recvfromNonBlock = nullptr;
    hooks->sendto = nullptr;
    hooks->sendtoNonBlock = nullptr;
    hooks->createSocket = TcpsockCreate;
    hooks->optionSet = SockOptionSetGeneral;
    hooks->optionGet = SockOptionGetGeneral;
}

__attribute__((constructor)) int TcpsockInit(void)
{
    int ret;
    struct SockCommHooks hooks;

    TcpsockRegisterSocketHooks(&hooks);
    ret = SockCommHooksReg(NET_TYPE_TCP, &hooks);
    return ret;
}

#ifdef __cplusplus
}
#endif
