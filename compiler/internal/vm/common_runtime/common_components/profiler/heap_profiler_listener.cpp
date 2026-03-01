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

#include "common_components/log/log.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_interfaces/profiler/heap_profiler_listener.h"

namespace common {
HeapProfilerListener &HeapProfilerListener::GetInstance()
{
    static HeapProfilerListener instance;
    return instance;
}

uint32_t HeapProfilerListener::RegisterMoveEventCb(const std::function<void(uintptr_t, uintptr_t, size_t)> &cb)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    moveEventCbs_.emplace(moveEventCbId_, cb);
    return moveEventCbId_++;
}

void HeapProfilerListener::UnRegisterMoveEventCb(uint32_t key)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    moveEventCbs_.erase(key);
}

void HeapProfilerListener::OnMoveEvent(uintptr_t fromObj, uintptr_t toObj, size_t size)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (const auto &pair : moveEventCbs_) {
        if (pair.second) {
            pair.second(fromObj, toObj, size);
        }
    }
}

void HeapProfilerListener::RegisterOutOfMemoryEventCb(const std::function<void(void *thread)> &cb)
{
    outOfMemoryEventCb_ = cb;
}
void HeapProfilerListener::OnOutOfMemoryEventCb()
{
    void *thread = nullptr;
    if (!IsGcThread()) {
        thread = Mutator::GetMutator()->GetThreadHolder()->GetJSThread();
    }
    outOfMemoryEventCb_(thread);
}
}  // namespace common