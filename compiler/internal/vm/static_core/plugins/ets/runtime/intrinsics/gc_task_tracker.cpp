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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/intrinsics/gc_task_tracker.h"

namespace ark::ets::intrinsics {

GCTaskTracker g_gcTaskTracker;           // NOLINT(fuchsia-statically-constructed-objects)
os::memory::Mutex GCTaskTracker::lock_;  // NOLINT(fuchsia-statically-constructed-objects)
bool GCTaskTracker::initialized_ = false;

/* static */
GCTaskTracker &GCTaskTracker::InitIfNeededAndGet(mem::GC *gc)
{
    os::memory::LockHolder lh(lock_);
    if (initialized_) {
        return g_gcTaskTracker;
    }
    gc->AddListener(&g_gcTaskTracker);
    initialized_ = true;
    return g_gcTaskTracker;
}

/* static */
bool GCTaskTracker::IsInitialized()
{
    os::memory::LockHolder lh(lock_);
    return initialized_;
}

void GCTaskTracker::AddTaskId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    taskIds_.push_back(id);
}

bool GCTaskTracker::HasId(uint64_t id)
{
    os::memory::LockHolder lock(lock_);
    return std::find(taskIds_.begin(), taskIds_.end(), id) != taskIds_.end();
}

void GCTaskTracker::SetCallbackForTask(uint32_t taskId, mem::Reference *callbackRef)
{
    callbackTaskId_ = taskId;
    callbackRef_ = callbackRef;
}

void GCTaskTracker::GCStarted(const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    currentTaskId_ = task.GetId();
}

void GCTaskTracker::GCPhaseStarted(mem::GCPhase phase)
{
    if (phase != mem::GCPhase::GC_PHASE_MARK || callbackRef_ == nullptr || currentTaskId_ != callbackTaskId_) {
        return;
    }
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *obj = reinterpret_cast<EtsObject *>(coroutine->GetPandaVM()->GetGlobalObjectStorage()->Get(callbackRef_));
    Value arg(obj->GetCoreType());
    os::memory::ReadLockHolder lock(*coroutine->GetPandaVM()->GetRendezvous()->GetMutatorLock());
    LambdaUtils::InvokeVoid(coroutine, obj);
}

void GCTaskTracker::GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                               [[maybe_unused]] size_t heapSize)
{
    RemoveId(task.GetId());
}

void GCTaskTracker::RemoveId(uint64_t id)
{
    currentTaskId_ = 0;
    if (id == callbackTaskId_ && callbackRef_ != nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ASSERT(coroutine != nullptr);
        coroutine->GetPandaVM()->GetGlobalObjectStorage()->Remove(callbackRef_);
        callbackRef_ = nullptr;
    }
    if (id != 0) {
        os::memory::LockHolder lock(lock_);
        auto it = std::find(taskIds_.begin(), taskIds_.end(), id);
        // There may be no such id if the corresponding GC has been triggered not by startGC
        if (it != taskIds_.end()) {
            taskIds_.erase(it);
        }
    }
}

}  // namespace ark::ets::intrinsics
