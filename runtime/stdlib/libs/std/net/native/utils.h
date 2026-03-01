/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODIRA_UTILS_H
#define CODIRA_UTILS_H

#include <stdint.h>
#if defined(_WIN32) && defined(__MINGW64__)
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

int CODE_SockOptionGet(int64_t sock, int level, int optname, void* optval, socklen_t* optlen);
int CODE_SockOptionSet(long long sock, int level, int optname, const void* optval, socklen_t optlen);

// copied from ytls
struct SockAddr {
    struct sockaddr_storage* sockaddr;
    socklen_t addrLen;
};
#endif // CODIRA_UTILS_H
