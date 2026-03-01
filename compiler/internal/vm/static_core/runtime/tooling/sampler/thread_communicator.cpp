/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/thread_communicator.h"

namespace ark::tooling::sampler {

bool ThreadCommunicator::IsPipeEmpty() const
{
    ASSERT(listenerPipe_[PIPE_READ_ID] != 0 && listenerPipe_[PIPE_WRITE_ID] != 0);

    struct pollfd pollFd = {listenerPipe_[PIPE_READ_ID], POLLIN, 0};
    return poll(&pollFd, 1, 0) == 0;
}

bool ThreadCommunicator::SendSample(const SampleInfo &sample) const
{
    ASSERT(listenerPipe_[PIPE_READ_ID] != 0 && listenerPipe_[PIPE_WRITE_ID] != 0);

    const void *buffer = reinterpret_cast<const void *>(&sample);
    ssize_t syscallResult = write(listenerPipe_[PIPE_WRITE_ID], buffer, sizeof(SampleInfo));
    if (syscallResult == -1) {
        return false;
    }
    LOG_IF(syscallResult != sizeof(SampleInfo), FATAL, PROFILER)
        << "unexpected sample write - sended " << syscallResult << " bytes";
    return true;
}

bool ThreadCommunicator::ReadSample(SampleInfo *sample) const
{
    ASSERT(listenerPipe_[PIPE_READ_ID] != 0 && listenerPipe_[PIPE_WRITE_ID] != 0);

    void *buffer = reinterpret_cast<void *>(sample);

    // NOTE(m.strizhak): optimize by reading several samples by one call
    ssize_t syscallResult = read(listenerPipe_[PIPE_READ_ID], buffer, sizeof(SampleInfo));
    if (syscallResult == -1) {
        return false;
    }
    LOG_IF(syscallResult != sizeof(SampleInfo), FATAL, PROFILER)
        << "unexpected sample read - received " << syscallResult << " bytes";
    return true;
}

}  // namespace ark::tooling::sampler
