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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_THREAD_COMMUNICATOR_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_THREAD_COMMUNICATOR_H

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/os/failure_retry.h"

#include "runtime/tooling/sampler/sample_info.h"

#ifdef PANDA_TARGET_MACOS
static int pipe2(int pipefd[2], int flags)
{
    int rc;
    int saved_errno;

    // Validate flags
    if (flags & ~(O_CLOEXEC | O_NONBLOCK)) {
        errno = EINVAL;
        return -1;
    }

    // Create pipe
    rc = pipe(pipefd);
    if (rc == -1) {
        return -1;
    }

    // Apply flags with error handling
    if (flags != 0) {
        if (flags & O_CLOEXEC) {
            if (fcntl(pipefd[0], F_SETFD, FD_CLOEXEC) == -1 || fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) == -1) {
                goto error;
            }
        }

        if (flags & O_NONBLOCK) {
            if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == -1 || fcntl(pipefd[1], F_SETFL, O_NONBLOCK) == -1) {
                goto error;
            }
        }
    }

    return 0;

error:
    saved_errno = errno;
    close(pipefd[0]);
    close(pipefd[1]);
    errno = saved_errno;
    return -1;
}
#endif

namespace ark::tooling::sampler {

namespace test {
class SamplerTest;
}  // namespace test

class ThreadCommunicator final {
public:
    static constexpr uint8_t PIPE_READ_ID {0};
    static constexpr uint8_t PIPE_WRITE_ID {1};

    ThreadCommunicator() = default;

    ~ThreadCommunicator()
    {
        for (int fd : listenerPipe_) {
            if (fd != -1) {
                if (PANDA_FAILURE_RETRY(::close(fd)) != 0) {
                    LOG(ERROR, PROFILER) << "Cannot close fd: " << fd;
                }
            }
        }
    }

    bool Init()
    {
        if (listenerPipe_[PIPE_READ_ID] != -1) {
            return true;
        }
        return pipe2(listenerPipe_.data(), O_CLOEXEC) != -1;
    }

    bool IsPipeEmpty() const;
    PANDA_PUBLIC_API bool SendSample(const SampleInfo &sample) const;
    PANDA_PUBLIC_API bool ReadSample(SampleInfo *sample) const;

    NO_COPY_SEMANTIC(ThreadCommunicator);
    NO_MOVE_SEMANTIC(ThreadCommunicator);

private:
    std::array<int, 2U> listenerPipe_ {-1, -1};

    friend class test::SamplerTest;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_THREAD_COMMUNICATOR_H
