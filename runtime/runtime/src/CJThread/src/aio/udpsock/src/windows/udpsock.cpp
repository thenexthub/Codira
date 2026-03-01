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
#include "schdfd_impl.h"
#include "securec.h"
#include "udpsock.h"

#ifdef __cplusplus
extern "C" {
#endif

SOCKET UdpsockCreate(int domain, int type, int protocol, int *socketError)
{
    SOCKET sockFd;
    int ret;
    if (type != SOCK_DGRAM) {
        *socketError = ERRNO_SOCK_ARG_INVALID;
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "udp socket type invalid: %d", type);
        return -1;
    }
    // create socket and set NONBLOCK, SOCK_CLOEXEC
    sockFd = WSASocket(domain, SOCK_DGRAM, protocol, nullptr,
                       0, WSA_FLAG_OVERLAPPED);
    if (sockFd == INVALID_SOCKET) {
        *socketError = WSAGetLastError();
        LOG_ERROR(*socketError, "udp socket failed, domain: %u, protocol: %d", domain, protocol);
        return -1;
    }
    ret = SchdfdRegisterAndNetpollAdd(sockFd);
    if (ret != 0) {
        *socketError = ret;
        LOG_ERROR(ret, "udpsock SchdfdRegisterAndNetpollAdd failed, fd: %d", sockFd);
        return -1;
    }

    LOG_INFO(0, "udp socket create success, fd: %d", sockFd);
    return sockFd;
}

int UdpsockDisconnect(SignedSocket connFd)
{
    struct sockaddr_storage peerAddr;
    socklen_t peerLen = sizeof(struct sockaddr_storage);
    int ret;

    memset_s(&peerAddr, sizeof(struct sockaddr_storage), 0, sizeof(struct sockaddr_storage));
    peerAddr.ss_family = AF_UNSPEC;

    ret = UdpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(&peerAddr), peerLen);
    if (ret != 0) {
        LOG_ERROR(ret, "UdpsockConnect failed");
        return ret;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
