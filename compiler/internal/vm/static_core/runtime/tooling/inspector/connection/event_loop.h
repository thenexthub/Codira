/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_EVENT_LOOP_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_EVENT_LOOP_H

#include <atomic>

#include "libarkbase/os/mutex.h"

namespace ark::tooling::inspector {
class EventLoop {
public:
    // Notify the running event loop to stop.
    bool Kill();

    // Run the event loop until paused.
    void Run();

    // Run at most one event loop handler, may block.
    virtual bool RunOne() = 0;

    // Pause the event loop. Wait for the current task to finish.
    void Pause() ACQUIRE_SHARED(taskExecution_)
    {
        taskExecution_.ReadLock();
    }

    // Notify the event loop to continue.
    void Continue() RELEASE_GENERIC(taskExecution_)
    {
        taskExecution_.Unlock();
    }

private:
    std::atomic<bool> running_ {false};
    os::memory::RWLock taskExecution_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_EVENT_LOOP_H
