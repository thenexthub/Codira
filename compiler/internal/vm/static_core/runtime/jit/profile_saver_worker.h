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
#ifndef PANDA_RUNTIME_PROFILE_SAVER_WORKER_H
#define PANDA_RUNTIME_PROFILE_SAVER_WORKER_H

#include <chrono>
#include "runtime/include/mem/allocator.h"
#include "libarkbase/taskmanager/task_queue_interface.h"
#include "libarkbase/taskmanager/task_manager.h"

namespace ark {
/// @brief Profile Saver worker
class ProfileSaverWorker {
public:
    explicit ProfileSaverWorker(mem::InternalAllocatorPtr internalAllocator);
    NO_COPY_SEMANTIC(ProfileSaverWorker);
    NO_MOVE_SEMANTIC(ProfileSaverWorker);

    ~ProfileSaverWorker()
    {
        taskmanager::TaskManager::DestroyTaskQueue<decltype(internalAllocator_->Adapter())>(profileSaverTaskQueue_);
    }

    void InitializeWorker();

    void FinalizeWorker()
    {
        JoinWorker();
    }

    void JoinWorker();

    bool IsWorkerJoined()
    {
        return profileSaverWorkerJoined_;
    }

    void EndTask();

    bool TryAddTask();

    void AddTask();

    void PreZygoteFork()
    {
        FinalizeWorker();
    }

    void PostZygoteFork()
    {
        InitializeWorker();
    }

    void WaitForFinishSaverTask();

private:
    taskmanager::TaskQueueInterface *profileSaverTaskQueue_ {nullptr};
    os::memory::Mutex taskQueueLock_;
    std::atomic<bool> profileSaverWorkerJoined_ {true};
    bool hasProfileSaverTaskInQueue_ GUARDED_BY(taskQueueLock_) {false};
    std::chrono::steady_clock::time_point lastSaveTime_ GUARDED_BY(taskQueueLock_);
    mem::InternalAllocatorPtr internalAllocator_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_PROFILE_SAVER_WORKER_H
