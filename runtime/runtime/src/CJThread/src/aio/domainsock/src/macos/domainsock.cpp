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

int DomainsockDisconnect(SignedSocket connFd)
{
    struct sockaddr_storage peerAddr;
    SchdpollEventType type = SHCDPOLL_WRITE;
    socklen_t peerLen = sizeof(struct sockaddr);
    socklen_t optLen = static_cast<socklen_t>(sizeof(int));
    int ret;
    int opt;
    if (getsockopt(connFd, SOL_SOCKET, SO_TYPE, &opt, &optLen) == -1) {
        ret = errno;
        LOG_ERROR(ret, "getsockopt failed, fd: %d", connFd);
        return ret;
    }
    
    if (opt != SOCK_DGRAM) {
        LOG_ERROR(ERRNO_SOCK_ARG_INVALID, "unix socket type isn't SOCK_DGRAM, fd: %d", connFd);
        return ERRNO_SOCK_ARG_INVALID;
    }
    ret = memset_s(&peerAddr, sizeof(struct sockaddr_storage), 0, sizeof(struct sockaddr_storage));
    if (ret != 0) {
        return ret;
    }
    peerAddr.ss_family = AF_UNSPEC;
    ret = SchdfdLock(connFd, type);
    if (ret != 0) {
        return ret;
    }
    ret = DomainsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(&peerAddr), peerLen, -1);
    if (ret != 0 && errno != ENOENT) {
        LOG_ERROR(ret, "DomainsockConnect failed");
        SchdfdUnlock(connFd, type);
        return ret;
    }
    SchdfdUnlock(connFd, type);
    return 0;
}

#ifdef __cplusplus
}
#endif
