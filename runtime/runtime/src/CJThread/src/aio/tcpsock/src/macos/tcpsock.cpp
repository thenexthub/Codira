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
#include "tcpsock.h"

#ifdef __cplusplus
extern "C" {
#endif

int TcpsockKeepAliveSet(int fd, const struct SockKeepAliveCfg *cfg)
{
    int ret = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &cfg->keepAlive, sizeof(unsigned int)) == -1) {
        ret = errno;
        LOG_ERROR(ret, "setsockopt failed, fd: %d, val: %u", fd, cfg->keepAlive);
        return ret;
    }

    // Setting it off does not require further configuration.
    if (cfg->keepAlive == 0) {
        return 0;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &cfg->idle, sizeof(unsigned int)) == -1) {
        ret = errno;
        LOG_ERROR(ret, "setsockopt failed, fd: %d, val: %u", fd, cfg->idle);
        return ret;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &cfg->interval, sizeof(unsigned int)) == -1) {
        ret = errno;
        LOG_ERROR(ret, "setsockopt failed, fd: %d, val: %u", fd, cfg->interval);
        return ret;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cfg->count, sizeof(unsigned int)) == -1) {
        ret = errno;
        LOG_ERROR(ret, "setsockopt failed, fd: %d, val: %u", fd, cfg->count);
        return ret;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
