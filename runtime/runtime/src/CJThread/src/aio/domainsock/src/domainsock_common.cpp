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


#include <unistd.h>
#include "log.h"
#include "sock_impl.h"
#include "schdfd_impl.h"
#include "securec.h"
#include "domainsock.h"

#ifdef __cplusplus
extern "C" {
#endif

int DomainsockCreate(int domain, int type, int protocol, int *socketError)
{
    int sockFd;
    int ret = 0;

    if (domain != AF_UNIX || (type != SOCK_STREAM && type != SOCK_DGRAM)) {
        *socketError = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "type:%d out of compliance", type);
        return -1;
    }

    // create domain socket and set NONBLOCK, SOCK_CLOEXEC.
    sockFd = SockCreateInternal(domain, type, protocol, socketError);
    if (sockFd == -1) {
        return -1;
    }
    ret = SchdfdRegister(sockFd);
    if (ret != 0) {
        *socketError = ret;
        LOG_ERROR(ret, "sockFd SchdfdRegister failed!");
        (void)close(sockFd);
        return -1;
    }
    LOG_INFO(0, "socket create success, fd: %d", sockFd);
    return sockFd;
}

int DomainsockBind(int fd, const struct sockaddr *addr, socklen_t addrLen)
{
    int ret;
    const struct sockaddr_un *domainsockAddr;

    domainsockAddr = reinterpret_cast<const struct sockaddr_un *>(addr);

    if (bind(fd, addr, addrLen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "bind failed, fd: %d, path: %s,addrlen: %u", fd, domainsockAddr->sun_path, addrLen);
        return ret;
    }

    return 0;
}

int DomainsockBindListen(int sockFd, const struct SockAddr *addr, int backlog)
{
    int ret;
    int opt;
    socklen_t optLen = static_cast<socklen_t>(sizeof(int));
    SchdpollEventType type = SHCDPOLL_READ;

    ret = SchdfdLock(sockFd, type);
    if (ret != 0) {
        return ret;
    }
    ret = DomainsockBind(sockFd, reinterpret_cast<const struct sockaddr *>(addr->sockaddr), addr->addrLen);
    if (ret != 0) {
        SchdfdUnlock(sockFd, type);
        return ret;
    }

    if (getsockopt(sockFd, SOL_SOCKET, SO_TYPE, &opt, &optLen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "getsockopt failed, fd: %d", sockFd);
        SchdfdUnlock(sockFd, type);
        return ret;
    }
    // Only domain sockets for byte streams need to execute the listen function.
    if (opt == SOCK_STREAM) {
        if (listen(sockFd, backlog) == -1) {
            ret = errno;
            LOG_ERROR(ret, "listen failed, fd: %d", sockFd);
            SchdfdUnlock(sockFd, type);
            return ret;
        }
        LOG_INFO(0, "listen success, fd: %d", sockFd);
    }
    ret = SchdfdNetpollAdd(sockFd);
    if (ret != 0) {
        LOG_ERROR(ret, "sockFd SchdfdNetpollAdd failed!");
        SchdfdUnlock(sockFd, type);
        return ret;
    }
    SchdfdUnlock(sockFd, type);
    return 0;
}

int DomainsockAccept(int inFd, int *outFd, struct SockAddr *addr, unsigned long long timeout)
{
    SchdpollEventType type = SHCDPOLL_READ;
    int domainFd;
    int fcntlErr = 0;
    socklen_t addrLen = 0;
    struct sockaddr *sockAddr = (addr == nullptr || addr->sockaddr == nullptr) ?
                                nullptr : reinterpret_cast<struct sockaddr *>(addr->sockaddr);
    socklen_t *addrLenPtr = (addr == nullptr || addr->sockaddr == nullptr) ? &addrLen : &(addr->addrLen);
    int ret = SchdfdLock(inFd, type);
    if (ret != 0) {
        return ret;
    }

    while (1) {
        domainFd = SockAcceptInternal(inFd, sockAddr, addrLenPtr, ret, fcntlErr);
        if (fcntlErr != 0) {
            SchdfdUnlock(inFd, type);
            return fcntlErr;
        }

        if (domainFd >= 0) {
            break;
        }

        if (ret != EAGAIN) {
            SchdfdUnlock(inFd, type);
            LOG_ERROR(ret, "domainsock accept failed, fd: %d", inFd);
            return ret;
        }

        LOG_INFO(0, "domainsock accept waiting, fd: %d", inFd);
        ret = (timeout == static_cast<unsigned long long>(-1)) ? SchdfdWaitInlock(inFd, type) :
              SchdfdWaitInlockTimeout(inFd, type, timeout);
        LOG_INFO(0, "domainsock accept wait over, fd: %d", inFd);

        if (ret != 0) {
            SchdfdUnlock(inFd, type);
            return ret;
        }
    }
    SchdfdUnlock(inFd, type);

    ret = SchdfdRegisterAndNetpollAdd(domainFd);
    if (ret != 0) {
        LOG_ERROR(ret, "domainsock SchdfdRegisterAndNetpollAdd failed, fd: %d", domainFd);
        return ret;
    }
    *outFd = domainFd;
    return 0;
}

int DomainsockConnect(int fd, const struct sockaddr *addr, socklen_t addrLen, unsigned long long timeout)
{
    socklen_t optLen = static_cast<socklen_t>(sizeof(int));
    SchdpollEventType type = SHCDPOLL_WRITE;
    int err;
    int opt;

    err = connect(fd, addr, addrLen);
    if (err == 0) {
        return SchdfdNetpollAdd(fd);
    }
    // The connection is in progress and need to wait for a writable event to complete it.
    // getsockopt obtains the connection result.
    if (errno == EINPROGRESS) {
        err = SchdfdNetpollAdd(fd);
        if (err != 0) {
            LOG_ERROR(err, "domain sockFd SchdfdNetpollAdd failed!");
            return err;
        }
        LOG_INFO(0, "domain connect waiting, fd: %d", fd);
        if (timeout == static_cast<unsigned long long>(-1)) {
            err = SchdfdWait(fd, type);
        } else {
            err = SchdfdWaitTimeout(fd, type, timeout);
        }
        LOG_INFO(0, "domain connect wait over, fd: %d", fd);

        if (err != 0) {
            return err;
        }

        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &optLen) == -1) {
            err = errno;
            LOG_ERROR(err, "domain getsockopt failed, fd: %d", fd);
            return err;
        }

        if (opt != 0) {
            LOG_ERROR(opt, "domain connect failed, fd: %d", fd);
            return opt;
        }
        return 0;
    }

    err = errno;
    LOG_ERROR(err, "domain connect failed, fd: %d", fd);
    return err;
}

int DomainsockBindConnect(int connFd, struct SockAddr *local, struct SockAddr *peer, unsigned long long timeout)
{
    int err;
    SchdpollEventType type = SHCDPOLL_WRITE;
    err = SchdfdLock(connFd, type);
    if (err != 0) {
        return err;
    }
    if (local != nullptr && local->sockaddr != nullptr) {
        err = DomainsockBind(connFd, reinterpret_cast<const struct sockaddr *>(local->sockaddr), local->addrLen);
        if (err != 0) {
            SchdfdUnlock(connFd, type);
            LOG_ERROR(err, "DomainsockBind failed");
            return err;
        }
    }

    err = DomainsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(peer->sockaddr), peer->addrLen, timeout);
    if (err != 0) {
        SchdfdUnlock(connFd, type);
        LOG_ERROR(err, "DomainsockConnect failed");
        return err;
    }
    SchdfdUnlock(connFd, type);
    return 0;
}

__attribute__((constructor)) int DomainsockInit(void)
{
    int ret;
    struct SockCommHooks hooks;

    hooks.bind = DomainsockBindListen;
    hooks.listen = nullptr;
    hooks.accept = DomainsockAccept;
    hooks.connect = DomainsockBindConnect;
    hooks.disconnect = DomainsockDisconnect;
    hooks.send = SockSendGeneral;
    hooks.sendNonBlock = nullptr;
    hooks.waitSend = nullptr;
    hooks.recv = SockRecvGeneral;
    hooks.recvNonBlock = nullptr;
    hooks.waitRecv = nullptr;
    hooks.close = SockCloseGeneral;
    hooks.shutdown = SockShutdownGeneral;
    hooks.keepAliveSet = nullptr;
    hooks.sockAddrGet = SockAddrGetGeneral;
    hooks.recvfrom = SockRecvfromGeneral;
    hooks.recvfromNonBlock = nullptr;
    hooks.sendto = SockSendtoGeneral;
    hooks.sendtoNonBlock = nullptr;
    hooks.createSocket = DomainsockCreate;
    hooks.optionSet = SockOptionSetGeneral;
    hooks.optionGet = SockOptionGetGeneral;

    ret = SockCommHooksReg(NET_TYPE_DOMAIN, &hooks);

    return ret;
}

#ifdef __cplusplus
}
#endif
