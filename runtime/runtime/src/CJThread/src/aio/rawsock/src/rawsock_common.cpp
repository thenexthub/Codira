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
#include "rawsock.h"
#if defined (MRT_LINUX) || defined (MRT_MACOS)
#include <unistd.h>
#include <net/if.h>
#include "log.h"
#include "schdfd_impl.h"
#include "securec.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (MRT_LINUX) || defined (MRT_MACOS)

int RawsockCreate(int domain, int type, int protocol, int *socketError)
{
    int sockFd;
    int ret = 0;
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
    LOG_INFO(0, "raw socket create success, fd: %d", sockFd);
    return sockFd;
}

int RawsockBind(int sockFd, const struct SockAddr *addr, int backlog)
{
    int ret;
    struct sockaddr *unixSockAddr = reinterpret_cast<struct sockaddr *>(addr->sockaddr);
    socklen_t addrLen = addr->addrLen;
    (void)backlog;
    if (bind(sockFd, unixSockAddr, addrLen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "bind failed, fd: %d, addrlen: %u", sockFd, addrLen);
        return ret;
    }
    LOG_INFO(0, "bind success, fd: %d", sockFd);

    return 0;
}

int RawsockListen(int sockFd, int backlog)
{
    int ret;
    if (listen(sockFd, backlog) == -1) {
        ret = errno;
        LOG_ERROR(ret, "listen failed, fd: %d", sockFd);
        return ret;
    }
    LOG_INFO(0, "listen success, fd: %d", sockFd);
    return 0;
}

int RawsockAccept(int inFd, int *outFd, struct SockAddr *addr, unsigned long long timeout)
{
    int fd;
    int fcntlErr = 0;
    socklen_t addrLen = 0;
    SchdpollEventType type = SHCDPOLL_READ;
    struct sockaddr *sockAddr = (addr == nullptr || addr->sockaddr == nullptr) ?
                                nullptr : reinterpret_cast<struct sockaddr *>(addr->sockaddr);
    socklen_t *addrLenPtr = (addr == nullptr || addr->sockaddr == nullptr) ? &addrLen : &(addr->addrLen);
    int ret = SchdfdLock(inFd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        fd = SockAcceptInternal(inFd, sockAddr, addrLenPtr, ret, fcntlErr);
        if (fcntlErr != 0) {
            SchdfdUnlock(inFd, type);
            return fcntlErr;
        }

        if (fd >= 0) {
            break;
        }

        if (ret != EAGAIN) {
            SchdfdUnlock(inFd, type);
            LOG_ERROR(ret, "raw socket accept failed, fd: %d", inFd);
            return ret;
        }
        ret = SchdfdNetpollAdd(inFd);
        if (ret != 0) {
            SchdfdUnlock(inFd, type);
            LOG_ERROR(ret, "SchdfdNetpollAdd failed!");
            return ret;
        }
        LOG_INFO(0, "raw socket accept waiting, fd: %d", inFd);
        ret = (timeout == static_cast<unsigned long long>(-1)) ? SchdfdWaitInlock(inFd, type) :
              SchdfdWaitInlockTimeout(inFd, type, timeout);
        LOG_INFO(0, "raw socket accept wait over, fd: %d", inFd);

        if (ret != 0) {
            SchdfdUnlock(inFd, type);
            return ret;
        }
    }
    SchdfdUnlock(inFd, type);

    ret = SchdfdRegisterAndNetpollAdd(fd);
    if (ret != 0) {
        LOG_ERROR(ret, "rawsock SchdfdRegisterAndNetpollAdd failed, fd: %d", fd);
        return ret;
    }
    *outFd = fd;
    return 0;
}

int RawsockConnect(int fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout)
{
    int opt;
    int ret;
    SchdpollEventType type = SHCDPOLL_WRITE;
    socklen_t optLen = static_cast<socklen_t>(sizeof(int));

    ret = connect(fd, addr, addrLen);
    if (ret == 0) {
        return SchdfdNetpollAdd(fd);
    }
    // The connection is in progress and need to wait for a writable event to complete it.
    // getsockopt obtains the connection result.
    if (errno == EINPROGRESS) {
        ret = SchdfdNetpollAdd(fd);
        if (ret != 0) {
            LOG_ERROR(ret, "rawsock SchdfdNetpollAdd failed!");
            return ret;
        }
        LOG_INFO(0, "rawsock connect waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            ret = SchdfdWaitInlock(fd, type);
        } else {
            ret = SchdfdWaitInlockTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "rawsock connect wait over, fd: %d", fd);

        if (ret != 0) {
            return ret;
        }

        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &optLen) == -1) {
            ret = errno;
            LOG_ERROR(ret, "rawsock getsockopt failed, fd: %d", fd);
            return ret;
        }

        if (opt != 0) {
            LOG_ERROR(opt, "rawsock connect failed, fd: %d", fd);
            return opt;
        }
        return 0;
    }

    ret = errno;
    LOG_ERROR(ret, "rawsock connect failed, fd: %d", fd);
    return ret;
}

int RawsockBindConnect(int connFd, struct SockAddr *local, struct SockAddr *peer, unsigned long long timeout)
{
    SchdpollEventType type = SHCDPOLL_WRITE;
    int ret;

    ret = SchdfdLock(connFd, type);
    if (ret != 0) {
        return ret;
    }

    if (local != nullptr && local->sockaddr != nullptr) {
        ret = RawsockBind(connFd, local, 0);
        if (ret != 0) {
            LOG_ERROR(ret, "RawsockBind failed");
            SchdfdUnlock(connFd, type);
            return ret;
        }
    }

    ret = RawsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen, timeout);
    if (ret != 0) {
        LOG_ERROR(ret, "RawsockConnect failed");
        SchdfdUnlock(connFd, type);
        return ret;
    }
    SchdfdUnlock(connFd, type);
    return 0;
}

int RawsockWait(int fd, SchdpollEventType type, unsigned long long timeout)
{
    int ret = SchdfdLock(fd, type);
    if (ret != 0) {
        return ret;
    }
    ret = SchdfdNetpollAdd(fd);
    if (ret != 0) {
        SchdfdUnlock(fd, type);
        LOG_ERROR(ret, "SchdfdNetpollAdd failed!");
        return ret;
    }
    LOG_INFO(0, "rawsock waiting, fd: %d", fd);
    if (timeout == static_cast<unsigned long long>(-1)) {
        ret = SchdfdWaitInlock(fd, type);
    } else {
        ret = SchdfdWaitInlockTimeout(fd, type, timeout);
    }
    LOG_INFO(0, "rawsock wait over, fd: %d", fd);
    SchdfdUnlock(fd, type);
    return ret;
}

int RawsockWaitSend(SignedSocket fd, unsigned long long timeout)
{
    return RawsockWait(fd, SHCDPOLL_WRITE, timeout);
}

int RawsockWaitRecv(SignedSocket fd, unsigned long long timeout)
{
    return RawsockWait(fd, SHCDPOLL_READ, timeout);
}

#endif

// Register socket hooks
void RawsockRegisterSocketHooks(struct SockCommHooks *hooks)
{
    hooks->createSocket = RawsockCreate;
    hooks->bind = RawsockBind;
    hooks->listen = RawsockListen;
    hooks->accept = RawsockAccept;
    hooks->connect = RawsockBindConnect;
    hooks->disconnect = RawsockDisconnect;
#ifdef MRT_WINDOWS
    hooks->send = SockSendGeneral;
    hooks->sendNonBlock = nullptr;
    hooks->waitSend = nullptr;
    hooks->recv = SockRecvGeneral;
    hooks->recvNonBlock = nullptr;
    hooks->waitRecv = nullptr;
    hooks->recvfrom = SockRecvfromGeneral;
    hooks->recvfromNonBlock = nullptr;
    hooks->sendto = SockSendtoGeneral;
    hooks->sendtoNonBlock = nullptr;
#else
    hooks->send = nullptr;
    hooks->sendNonBlock = SockSendNonBlockGeneral;
    hooks->waitSend = RawsockWaitSend;
    hooks->recv = nullptr;
    hooks->recvNonBlock = SockRecvNonBlockGeneral;
    hooks->waitRecv = RawsockWaitRecv;
    hooks->recvfrom = nullptr;
    hooks->recvfromNonBlock = SockRecvfromNonBlockGeneral;
    hooks->sendto = nullptr;
    hooks->sendtoNonBlock = SockSendtoNonBlockGeneral;
#endif
    hooks->close = SockCloseGeneral;
    hooks->shutdown = SockShutdownGeneral;
    hooks->keepAliveSet = nullptr;
    hooks->sockAddrGet = SockAddrGetGeneral;
    hooks->optionSet = SockOptionSetGeneral;
    hooks->optionGet = SockOptionGetGeneral;
}

__attribute__((constructor)) int RawsockInit(void)
{
    int ret;
    struct SockCommHooks hooks;

    RawsockRegisterSocketHooks(&hooks);
    ret = SockCommHooksReg(NET_TYPE_RAW, &hooks);
    return ret;
}

#ifdef __cplusplus
}
#endif
