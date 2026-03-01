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
#include "schdfd_impl.h"
#include "udpsock.h"

#ifdef __cplusplus
extern "C" {
#endif

int UdpsockDisconnect(SignedSocket connFd)
{
    struct sockaddr_storage peerAddr;
    socklen_t peerLen = sizeof(struct sockaddr);
    int ret = 0;

    ret = memset_s(&peerAddr, sizeof(struct sockaddr_storage), 0, sizeof(struct sockaddr_storage));
    if (ret != 0) {
        return ret;
    }
    peerAddr.ss_family = AF_UNSPEC;

    ret = UdpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(&peerAddr), peerLen);
    if (ret != 0 && errno != EAFNOSUPPORT) {
        LOG_ERROR(ret, "UdpsockConnect failed");
        return ret;
    }
    return 0;
}

int UdpsockDisconnectForIPv6(SignedSocket connFd)
{
    struct sockaddr_in6 peerAddr;
    socklen_t peerLen = sizeof(struct sockaddr_in6);
    int ret = 0;

    ret = memset_s(&peerAddr, sizeof(struct sockaddr_in6), 0, sizeof(struct sockaddr_in6));
    if (ret != 0) {
        return ret;
    }
    peerAddr.sin6_family = AF_UNSPEC;

    ret = UdpsockConnect(connFd, reinterpret_cast<const struct sockaddr *>(&peerAddr), peerLen);
    if (ret != 0 && errno != EAFNOSUPPORT) {
        LOG_ERROR(ret, "UdpsockConnect failed");
        return ret;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
