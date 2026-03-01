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


#include <cstdio>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>
#include "mrt_syscall.h"
#include "syscall_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

int SyscallReadv(int fileds, const struct iovec *iov, int iovcnt)
{
    int res;
    SyscallEnter();
    res = readv(fileds, iov, iovcnt);
    SyscallExit();
    return res;
}

int SyscallWritev(int fileds, const struct iovec *iov, int iovcnt)
{
    int res;
    SyscallEnter();
    res = writev(fileds, iov, iovcnt);
    SyscallExit();
    return res;
}

int SyscallFcntl(int fd, int cmd, ...)
{
    va_list argList;
    void *arg = nullptr;
    int res;

    SyscallEnter();
    va_start(argList, cmd);
    arg = va_arg(argList, void *);
    va_end(argList);
    res = fcntl(fd, cmd, arg);
    SyscallExit();
    return res;
}

int SyscallDup(int oldfd)
{
    int res;
    SyscallEnter();
    res = dup(oldfd);
    SyscallExit();
    return res;
}

int SyscallDup3(int oldfd, int newfd, int flags)
{
    int res;
    SyscallEnter();
    res = dup3(oldfd, newfd, flags);
    SyscallExit();
    return res;
}

int SyscallNanosleep(const struct timespec *req, struct timespec *rem)
{
    int res;
    SyscallEnter();
    res = nanosleep(req, rem);
    SyscallExit();
    return res;
}

int SyscallConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int res;
    SyscallEnter();
    res = connect(sockfd, addr, addrlen);
    SyscallExit();
    return res;
}

int SyscallRecv(int sockfd, void *buf, size_t len, int flags)
{
    int res;
    SyscallEnter();
    res = recv(sockfd, buf, len, flags);
    SyscallExit();
    return res;
}

int SyscallRecvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *srcAddr, socklen_t *addrlen)
{
    int res;
    SyscallEnter();
    res = recvfrom(sockfd, buf, len, flags, srcAddr, addrlen);
    SyscallExit();
    return res;
}

int SyscallRecvmsg(int sockfd, struct msghdr *msg, int flags)
{
    int res;
    SyscallEnter();
    res = recvmsg(sockfd, msg, flags);
    SyscallExit();
    return res;
}

int SyscallSend(int sockfd, const void *buf, size_t len, int flags)
{
    int res;
    SyscallEnter();
    res = send(sockfd, buf, len, flags);
    SyscallExit();
    return res;
}

int SyscallSendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *destAddr, socklen_t addrlen)
{
    int res;
    SyscallEnter();
    res = sendto(sockfd, buf, len, flags, destAddr, addrlen);
    SyscallExit();
    return res;
}

int SyscallSendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    int res;
    SyscallEnter();
    res = sendmsg(sockfd, msg, flags);
    SyscallExit();
    return res;
}

int SyscallGetsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    int res;
    SyscallEnter();
    res = getsockopt(sockfd, level, optname, optval, optlen);
    SyscallExit();
    return res;
}

int SyscallSetsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int res;
    SyscallEnter();
    res = setsockopt(sockfd, level, optname, optval, optlen);
    SyscallExit();
    return res;
}

int SyscallDup2(int oldfd, int newfd)
{
    int ret;

    SyscallEnter();
    ret = dup2(oldfd, newfd);
    SyscallExit();
    return ret;
}

int SyscallPoll(struct pollfd *fds, unsigned long nfds, int timeout)
{
    int ret;

    SyscallEnter();
    ret = poll(fds, nfds, timeout);
    SyscallExit();
    return ret;
}

int SyscallSelect(int nfds, fd_set *__restrict readfds, fd_set *__restrict writefds, fd_set *__restrict exceptfds,
                  struct timeval *__restrict timeout)
{
    int ret;

    SyscallEnter();
    ret = select(nfds, readfds, writefds, exceptfds, timeout);
    SyscallExit();
    return ret;
}

#ifdef __cplusplus
}
#endif
