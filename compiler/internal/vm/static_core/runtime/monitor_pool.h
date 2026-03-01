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
#ifndef PANDA_RUNTIME_MONITOR_POOL_H_
#define PANDA_RUNTIME_MONITOR_POOL_H_

#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/mark_word.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/monitor.h"

namespace ark {

class MonitorPool {
public:
    // Likely, we do not need to copy monitor pool
    NO_COPY_SEMANTIC(MonitorPool);
    NO_MOVE_SEMANTIC(MonitorPool);

    static constexpr Monitor::MonitorId MAX_MONITOR_ID = MarkWord::MONITOR_POINTER_MAX_COUNT;

    template <class Callback>
    void EnumerateMonitors(const Callback &cb)
    {
        os::memory::LockHolder lock(poolLock_);
        for (auto &iter : monitors_) {
            if (!cb(iter.second)) {
                break;
            }
        }
    }

    template <class Callback>
    void DeflateMonitorsWithCallBack(const Callback &cb)
    {
        os::memory::LockHolder lock(poolLock_);
        for (auto monitorIter = monitors_.begin(); monitorIter != monitors_.end();) {
            auto monitor = monitorIter->second;
            if (cb(monitor) && monitor->DeflateInternal()) {
                monitorIter = monitors_.erase(monitorIter);
                allocator_->Delete(monitor);
            } else {
                monitorIter++;
            }
        }
    }

    explicit MonitorPool(mem::InternalAllocatorPtr allocator) : monitors_(allocator->Adapter())
    {
        lastId_ = 0;
        allocator_ = allocator;
    }

    ~MonitorPool()
    {
#if defined(PANDA_TSAN_ON)
        // There is a potential false data race between destroying monitors and daemon threads which are got stuck in a
        // deadlock. The race is reported for accesses in Mutex::Lock() before going to endless sleep on futex call in
        // a deadlock. In this case the data race is reported with the main thread, which destroys the mutex.
        if (os::memory::Mutex::DoNotCheckOnTerminationLoop()) {
            TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        }
#endif
        for (auto &iter : monitors_) {
            if (iter.second != nullptr) {
                allocator_->Delete(iter.second);
            }
        }
#if defined(PANDA_TSAN_ON)
        if (os::memory::Mutex::DoNotCheckOnTerminationLoop()) {
            TSAN_ANNOTATE_IGNORE_WRITES_END();
        }
#endif
    }

    Monitor *CreateMonitor(ObjectHeader *obj);

    Monitor *LookupMonitor(Monitor::MonitorId id);

    void FreeMonitor(Monitor::MonitorId id);

    void DeflateMonitors();

    void ReleaseMonitors(MTManagedThread *thread);

    PandaSet<Monitor::MonitorId> GetEnteredMonitorsIds(MTManagedThread *thread);

private:
    mem::InternalAllocatorPtr allocator_;
    // Lock for private data protection.
    os::memory::Mutex poolLock_;

    Monitor::MonitorId lastId_ GUARDED_BY(poolLock_);
    PandaUnorderedMap<Monitor::MonitorId, Monitor *> monitors_ GUARDED_BY(poolLock_);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_MONITOR_POOL_H_
