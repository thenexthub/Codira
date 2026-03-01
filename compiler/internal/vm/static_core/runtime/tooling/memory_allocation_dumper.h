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
#ifndef PANDA_RUNTIME_TOOLING_MEM_ALLOC_DUMPER_H
#define PANDA_RUNTIME_TOOLING_MEM_ALLOC_DUMPER_H

#include <functional>
#include <memory>
#include <string_view>

#include "pt_hooks_wrapper.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/mem/panda_containers.h"
#include "include/runtime_notification.h"
#include "include/tooling/debug_interface.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/span.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/tooling/debug_interface.h"
#include "runtime/thread_manager.h"

namespace ark::tooling {
class MemoryAllocationDumper : public RuntimeListener {
public:
    explicit MemoryAllocationDumper(const PandaString &dumpFile, const Runtime *runtime) : runtime_(runtime)
    {
        dumpstream_.open(dumpFile.c_str(), std::ios::out);
        runtime_->GetNotificationManager()->AddListener(this, DUMP_EVENT_MASK);
    }

    ~MemoryAllocationDumper() override
    {
        runtime_->GetNotificationManager()->RemoveListener(this, DUMP_EVENT_MASK);
        dumpstream_.close();
    }

    // RuntimeListener methods
    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, ManagedThread *thread, size_t size) override
    {
        if (thread == nullptr) {
            thread = ManagedThread::GetCurrent();
            if (thread == nullptr) {
                DumpNotAllocEvent("NULLPTR_THREAD", "ObjectAlloc");
            }
        }
        ASSERT(thread == ManagedThread::GetCurrent());

        PandaString klassName;
        if (klass->IsDynamicClass()) {
            klassName = "Dynamic";
        } else {
            klassName = ClassHelper::GetName<PandaString>(static_cast<Class *>(klass)->GetDescriptor());
        }

        DumpAllocation("NO_ERROR", "ObjectAlloc", klassName, object, size);
    }

    void GarbageCollectorStart() override
    {
        DumpNotAllocEvent("NO_ERROR", "GarbageCollectorStart");
    }

    void GarbageCollectorFinish() override
    {
        DumpNotAllocEvent("NO_ERROR", "GarbageCollectorFinish");
    }

private:
    // Add one func to use lock in right way
    void DumpAllocation(const char *err, const char *caller, const PandaString &klassName, ObjectHeader *object,
                        size_t size)
    {
        os::memory::LockHolder lock(lock_);
        const void *rawptr = static_cast<const void *>(object);
        dumpstream_ << err << "," << time::GetCurrentTimeInMicros() << "," << caller << "," << klassName << ","
                    << rawptr << "," << size << std::endl;
    }

    void DumpNotAllocEvent(const char *err, const char *caller)
    {
        os::memory::LockHolder lock(lock_);
        dumpstream_ << err << "," << time::GetCurrentTimeInMicros() << "," << caller << std::endl;
    }

    static constexpr uint32_t DUMP_EVENT_MASK = RuntimeNotificationManager::Event::GARBAGE_COLLECTOR_EVENTS |
                                                RuntimeNotificationManager::Event::ALLOCATION_EVENTS;

    const Runtime *runtime_;
    std::ofstream dumpstream_;
    os::memory::Mutex lock_;

    NO_COPY_SEMANTIC(MemoryAllocationDumper);
    NO_MOVE_SEMANTIC(MemoryAllocationDumper);
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_MEM_ALLOC_DUMPER_H
