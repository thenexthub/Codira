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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_TASK_TRACKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_TASK_TRACKER_H

#include "runtime/mem/gc/gc.h"

namespace ark::ets::intrinsics {

/**
 * Class tracks GC tasks already processed by GC.
 * Also the class tracks concurrent mark GC phase and calls
 * the callback if it specified.
 */
class GCTaskTracker : public mem::GCListener {
public:
    static GCTaskTracker &InitIfNeededAndGet(mem::GC *gc);
    static bool IsInitialized();

    void AddTaskId(uint64_t id);
    bool HasId(uint64_t id);
    void SetCallbackForTask(uint32_t taskId, mem::Reference *callbackRef);
    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCPhaseStarted(mem::GCPhase phase) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    void RemoveId(uint64_t id);

private:
    static bool initialized_;
    static os::memory::Mutex lock_;

    std::vector<uint64_t> taskIds_ GUARDED_BY(lock_);
    uint32_t currentTaskId_ = 0;
    uint32_t callbackTaskId_ = 0;
    mem::Reference *callbackRef_ = nullptr;
};

}  // namespace ark::ets::intrinsics

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_TASK_TRACKER_H
