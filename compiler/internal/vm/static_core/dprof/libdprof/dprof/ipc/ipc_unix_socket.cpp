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

#include "ipc_unix_socket.h"

#include "libarkbase/os/failure_retry.h"
#include "libarkbase/utils/logger.h"
#include "securec.h"

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ark::dprof::ipc {
constexpr char SOCKET_NAME[] = "\0dprof.socket";  // NOLINT(modernize-avoid-c-arrays)
static_assert(sizeof(static_cast<sockaddr_un *>(nullptr)->sun_path) >= sizeof(SOCKET_NAME), "Socket name too large");

os::unique_fd::UniqueFd CreateUnixServerSocket(int backlog)
{
    os::unique_fd::UniqueFd sock(PANDA_FAILURE_RETRY(::socket(AF_UNIX, SOCK_STREAM, 0)));

    int opt = 1;
    if (PANDA_FAILURE_RETRY(::setsockopt(sock.Get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) == -1) {
        PLOG(ERROR, DPROF) << "setsockopt() failed";
        return os::unique_fd::UniqueFd();
    }

    struct sockaddr_un serverAddr {};
    if (memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr)) != EOK) {
        PLOG(ERROR, DPROF) << "CreateUnixServerSocket memset_s failed";
        UNREACHABLE();
    }
    serverAddr.sun_family = AF_UNIX;
    if (memcpy_s(serverAddr.sun_path, sizeof(SOCKET_NAME), SOCKET_NAME, sizeof(SOCKET_NAME)) != EOK) {
        PLOG(ERROR, DPROF) << "CreateUnixServerSocket memcpy_s failed";
        UNREACHABLE();
    }
    auto *sockAddr = reinterpret_cast<sockaddr *>(&serverAddr);
    if (PANDA_FAILURE_RETRY(::bind(sock.Get(), sockAddr, sizeof(serverAddr))) == -1) {
        PLOG(ERROR, DPROF) << "bind() failed";
        return os::unique_fd::UniqueFd();
    }

    if (::listen(sock.Get(), backlog) == -1) {
        PLOG(ERROR, DPROF) << "listen() failed";
        return os::unique_fd::UniqueFd();
    }

    return sock;
}

os::unique_fd::UniqueFd CreateUnixClientSocket()
{
    os::unique_fd::UniqueFd sock(PANDA_FAILURE_RETRY(::socket(AF_UNIX, SOCK_STREAM, 0)));
    if (!sock.IsValid()) {
        PLOG(ERROR, DPROF) << "socket() failed";
        return os::unique_fd::UniqueFd();
    }

    struct sockaddr_un serverAddr {};
    if (memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr)) != EOK) {
        PLOG(ERROR, DPROF) << "CreateUnixClientSocket memset_s failed";
        UNREACHABLE();
    }
    serverAddr.sun_family = AF_UNIX;
    if (memcpy_s(serverAddr.sun_path, sizeof(SOCKET_NAME), SOCKET_NAME, sizeof(SOCKET_NAME)) != EOK) {
        PLOG(ERROR, DPROF) << "CreateUnixClientSocket memcpy_s failed";
        UNREACHABLE();
    }
    auto *sockAddr = reinterpret_cast<sockaddr *>(&serverAddr);
    if (PANDA_FAILURE_RETRY(::connect(sock.Get(), sockAddr, sizeof(serverAddr))) == -1) {
        PLOG(ERROR, DPROF) << "connect() failed";
        return os::unique_fd::UniqueFd();
    }

    return sock;
}

bool SendAll(int fd, const void *buf, int len)
{
    const char *p = reinterpret_cast<const char *>(buf);
    int total = 0;
    while (total < len) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        int n = PANDA_FAILURE_RETRY(::send(fd, p + total, len, 0));
        if (n == -1) {
            PLOG(ERROR, DPROF) << "send() failed";
            return false;
        }
        total += n;
        len -= n;
    }
    return true;
}

bool WaitDataTimeout(int fd, int timeoutMs)
{
    struct pollfd pfd {};
    pfd.fd = fd;
    pfd.events = POLLIN;

    int rc = PANDA_FAILURE_RETRY(::poll(&pfd, 1, timeoutMs));
    if (rc == 1) {
        // Success
        return true;
    }
    if (rc == -1) {
        // Error
        PLOG(ERROR, DPROF) << "poll() failed";
        return false;
    }
    if (rc == 0) {
        // Timeout
        LOG(ERROR, DPROF) << "Timeout, cannot recv data";
        return false;
    }

    UNREACHABLE();
    return false;
}

int RecvTimeout(int fd, void *buf, int len, int timeoutMs)
{
    if (!WaitDataTimeout(fd, timeoutMs)) {
        LOG(ERROR, DPROF) << "Cannot get access to data";
        return -1;
    }

    int n = PANDA_FAILURE_RETRY(::recv(fd, buf, len, 0));
    if (n == -1) {
        PLOG(ERROR, DPROF) << "Cannot recv data, len=" << len;
        return -1;
    }
    if (n == 0) {
        // socket was closed
        return 0;
    }
    if (n != len) {
        LOG(ERROR, DPROF) << "Cannot recv data id, len=" << len << " n=" << n;
        return -1;
    }

    return len;
}
}  // namespace ark::dprof::ipc
