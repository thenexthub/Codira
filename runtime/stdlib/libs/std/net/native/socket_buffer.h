/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODIRA_SOCKET_BUFFER_H
#define CODIRA_SOCKET_BUFFER_H

#include "utils.h"

#if defined(__MINGW64__)
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

int32_t CODE_MRT_SockSend(long long sock, const char* buf, unsigned int len, int flags);
int32_t CODE_MRT_SockSendto(long long sock, const char* buf, unsigned int len, int flags, const struct SockAddr* addr);
int32_t CODE_MRT_SockSendTimeout(long long sock, const char* buf, unsigned int len, int flags, unsigned long long times);
int32_t CODE_MRT_SockRecv(long long sock, const char* buf, unsigned int len, int flags);
int32_t CODE_MRT_SockRecvTimeout(long long sock, const char* buf, unsigned int len, int flags, unsigned long long times);
int32_t CODE_MRT_SockRecvfromTimeout(
    long long sock, void* buf, unsigned int len, int flags, struct SockAddr* addr, unsigned long long timeout);
int32_t CODE_MRT_SockClose(long long sock);
int32_t CODE_SockShutdown(long long sock);

#endif // CODIRA_SOCKET_BUFFER_H
