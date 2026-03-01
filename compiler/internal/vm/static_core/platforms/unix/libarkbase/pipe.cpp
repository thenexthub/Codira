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

#include "libarkbase/os/pipe.h"
#include "libarkbase/os/failure_retry.h"

#include <vector>
#include <array>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

namespace ark::os {

std::pair<UniqueFd, UniqueFd> CreatePipe()
{
    constexpr size_t FD_NUM = 2;
    std::array<int, FD_NUM> fds {};
    // NOLINTNEXTLINE(android-cloexec-pipe)
    if (PANDA_FAILURE_RETRY(pipe(fds.data())) == -1) {
        return std::pair<UniqueFd, UniqueFd>();
    }
    return std::pair<UniqueFd, UniqueFd>(UniqueFd(fds[0]), UniqueFd(fds[1]));
}

int SetFdNonblocking(const UniqueFd &fd)
{
    size_t flags;
#ifdef O_NONBLOCK
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int res = fcntl(fd.Get(), F_GETFL, 0);
    if (res < 0) {
        flags = 0;
    } else {
        flags = static_cast<size_t>(res);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-signed-bitwise)
    return fcntl(fd.Get(), F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#endif
}

Expected<size_t, Error> ReadFromPipe(const UniqueFd &pipeFd, void *buf, size_t size)
{
    ssize_t bytesRead = PANDA_FAILURE_RETRY(read(pipeFd.Get(), buf, size));
    if (bytesRead < 0) {
        return Unexpected(Error(errno));
    }
    return {static_cast<size_t>(bytesRead)};
}

Expected<size_t, Error> WriteToPipe(const UniqueFd &pipeFd, const void *buf, size_t size)
{
    ssize_t bytesWritten = PANDA_FAILURE_RETRY(write(pipeFd.Get(), buf, size));
    if (bytesWritten < 0) {
        return Unexpected(Error(errno));
    }
    return {static_cast<size_t>(bytesWritten)};
}

Expected<size_t, Error> WaitForEvent(const UniqueFd *handles, size_t size, EventType type)
{
    uint16_t pollEvents;
    switch (type) {
        case EventType::READY:
            pollEvents = POLLIN;
            break;

        default:
            return Unexpected(Error("Unknown event type"));
    }

    // Initialize poll set
    std::vector<pollfd> pollfds(size);
    for (size_t i = 0; i < size; i++) {
        pollfds[i].fd = handles[i].Get();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pollfds[i].events = static_cast<int16_t>(pollEvents);
    }

    while (true) {
        int res = PANDA_FAILURE_RETRY(poll(pollfds.data(), size, -1));
        if (res == -1) {
            return Unexpected(Error(errno));
        }

        for (size_t i = 0; i < size; i++) {
            if ((static_cast<size_t>(pollfds[i].revents) & pollEvents) == pollEvents) {
                return Expected<size_t, Error>(i);
            }
        }
    }
}

std::optional<Error> Dup2(const UniqueFd &source, const UniqueFd &target)
{
    if (!source.IsValid()) {
        return Error("Source fd is invalid");
    }
    if (PANDA_FAILURE_RETRY(dup2(source.Get(), target.Get())) == -1) {
        return Error(errno);
    }
    return {};
}

}  // namespace ark::os
