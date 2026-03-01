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

#include "runtime/jit/profile_saver_worker.h"
#include "include/runtime.h"
#include "runtime/jit/profiling_saver.h"
#include "libarkbase/taskmanager/task_manager.h"
#include "libarkbase/utils/logger.h"

namespace ark {

ProfileSaverWorker::ProfileSaverWorker(mem::InternalAllocatorPtr internalAllocator)
    : internalAllocator_(internalAllocator)
{
    os::memory::LockHolder lock(taskQueueLock_);
    profileSaverTaskQueue_ = taskmanager::TaskManager::CreateTaskQueue<decltype(internalAllocator_->Adapter())>(
        taskmanager::MIN_QUEUE_PRIORITY);
    lastSaveTime_ = std::chrono::steady_clock::now();
    hasProfileSaverTaskInQueue_ = false;
    profileSaverWorkerJoined_ = false;
    ASSERT(profileSaverTaskQueue_ != nullptr);
}

void ProfileSaverWorker::InitializeWorker()
{
    os::memory::LockHolder lock(taskQueueLock_);
    lastSaveTime_ = std::chrono::steady_clock::now();
    profileSaverWorkerJoined_ = false;
}

void ProfileSaverWorker::JoinWorker()
{
    {
        os::memory::LockHolder lock(taskQueueLock_);
        profileSaverWorkerJoined_ = true;
    }
    profileSaverTaskQueue_->WaitBackgroundTasks();
}

void ProfileSaverWorker::WaitForFinishSaverTask()
{
    profileSaverTaskQueue_->WaitBackgroundTasks();
}

void ProfileSaverWorker::EndTask()
{
    os::memory::LockHolder lock(taskQueueLock_);
    hasProfileSaverTaskInQueue_ = false;
}

bool ProfileSaverWorker::TryAddTask()
{
    if (profileSaverWorkerJoined_) {
        return false;
    }

    {
        os::memory::LockHolder lock(taskQueueLock_);
        auto timestamp = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - lastSaveTime_).count();
        if (duration < Runtime::GetCurrent()->GetOptions().GetProfilesaverSleepingTimeMs()) {
            return false;
        }
        lastSaveTime_ = timestamp;
        if (hasProfileSaverTaskInQueue_) {
            LOG(INFO, RUNTIME) << "[profile_saver] already has task in queue.";
            return false;
        }
        hasProfileSaverTaskInQueue_ = true;
    }
    LOG(INFO, RUNTIME) << "[profile_saver] profile saver task created.";
    AddTask();
    return true;
}

void ProfileSaverWorker::AddTask()
{
    profileSaverTaskQueue_->AddBackgroundTask([this]() mutable {
        auto instance = Runtime::GetCurrent();
        if (instance->GetClassLinker()->GetAotManager()->HasProfiledMethods()) {
            LOG(INFO, RUNTIME) << "[profile_saver] increamental saving profile data...";
            ProfilingSaver profileSaver;
            auto classCtxStr = instance->GetClassLinker()->GetAotManager()->GetBootClassContext() + ":" +
                               instance->GetClassLinker()->GetAotManager()->GetAppClassContext();
            auto &profiledMethods = instance->GetClassLinker()->GetAotManager()->GetProfiledMethods();
            auto profiledMethodsFinal = instance->GetClassLinker()->GetAotManager()->GetProfiledMethodsFinal();
            auto savingPath = PandaString(instance->GetOptions().GetProfileOutput());
            auto profiledPandaFiles = instance->GetClassLinker()->GetAotManager()->GetProfiledPandaFiles();
            profileSaver.SaveProfile(savingPath, classCtxStr, profiledMethods, profiledMethodsFinal,
                                     profiledPandaFiles);
        } else {
            LOG(INFO, RUNTIME) << "[profile_saver] No profiled method, skip saver task.";
        }
        this->EndTask();
    });
}
}  // namespace ark
