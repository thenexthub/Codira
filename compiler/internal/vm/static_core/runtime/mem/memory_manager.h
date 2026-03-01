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

#ifndef PANDA_MEM_MEMORY_MANAGER_H
#define PANDA_MEM_MEMORY_MANAGER_H

#include <libarkbase/macros.h>
#include <libarkbase/mem/mem.h>
#include <runtime/mem/gc/gc_types.h>
#include <runtime/mem/heap_manager.h>
#include "runtime/mem/lock_config_helper.h"

namespace ark {
class RuntimeOptions;
}  // namespace ark
namespace ark::mem {

class GCStats;
class GCTrigger;
class GCSettings;
class GCTriggerConfig;
class Allocator;

/*
 * Relations between VMs, MemoryManager and Runtime:
 *
 * +-----------------------------------------------+
 * |                                               |
 * |                  Runtime                      |
 * |                                               |
 * |  +----------+  +----------+     +----------+  |
 * |  |          |  |          |     |          |  |
 * |  |   VM_0   |  |   VM_1   |     |   VM_N   |  |
 * |  |          |  |          |     |          |  |
 * |  |          |  |          | ... |          |  |
 * |  |  +----+  |  |  +----+  |     |  +----+  |  |
 * |  |  |MM_0|  |  |  |MM_1|  |     |  |MM_N|  |  |
 * |  |  +----+  |  |  +----+  |     |  +----+  |  |
 * |  +----------+  +----------+     +----------+  |
 * |         \           |            /            |
 * |          \          |           /             |
 * |           +--------------------+              |
 * |           | Internal Allocator |              |
 * |           +--------------------+              |
 * +-----------------------------------------------+
 */

/**
 * A class that encapsulates components for working with memory.
 * Each VM is allocated its own instance.
 */
class MemoryManager {
public:
    struct HeapOptions {
        HeapManager::IsObjectFinalizebleFunc isObjectFinalizebleFunc;
        HeapManager::RegisterFinalizeReferenceFunc registerFinalizeReferenceFunc;
        uint32_t maxGlobalRefSize;
        bool isGlobalReferenceSizeCheckEnabled;
        MTModeT multithreadingMode;
        bool isUseTlabForAllocations;
        bool isStartAsZygote;
    };

    // CC-OFFNXT(G.FUN.01-CPP) solid logic, the fix will degrade the readability and maintenance of the code
    static MemoryManager *Create(const LanguageContext &ctx, InternalAllocatorPtr internalAllocator, GCType gcType,
                                 const GCSettings &gcSettings, const GCTriggerConfig &gcTriggerConfig,
                                 const HeapOptions &heapOptions);
    static void Destroy(MemoryManager *mm);

    NO_COPY_SEMANTIC(MemoryManager);
    NO_MOVE_SEMANTIC(MemoryManager);

    void PreStartup();
    void PreZygoteFork();
    void PostZygoteFork();
    void InitializeGC(PandaVM *vm);
    PANDA_PUBLIC_API void StartGC();
    void StopGC();

    void Finalize();

    HeapManager *GetHeapManager()
    {
        ASSERT(heapManager_ != nullptr);
        return heapManager_;
    }

    GC *GetGC() const
    {
        ASSERT(gc_ != nullptr);
        return gc_;
    }

    GCTrigger *GetGCTrigger()
    {
        ASSERT(gcTrigger_ != nullptr);
        return gcTrigger_;
    }

    GCStats *GetGCStats()
    {
        ASSERT(gcStats_ != nullptr);
        return gcStats_;
    }

    GlobalObjectStorage *GetGlobalObjectStorage() const
    {
        ASSERT(globalObjectStorage_ != nullptr);
        return globalObjectStorage_;
    }

    MemStatsType *GetMemStats()
    {
        ASSERT(memStats_ != nullptr);
        return memStats_;
    }

private:
    // CC-OFFNXT(G.FUN.01-CPP) solid logic, the fix will degrade the readability and maintenance of the code
    explicit MemoryManager(InternalAllocatorPtr internalAllocator, HeapManager *heapManager, GC *gc,
                           GCTrigger *gcTrigger, GCStats *gcStats, MemStatsType *memStats,
                           GlobalObjectStorage *globalObjectStorage)
        : internalAllocator_(internalAllocator),
          heapManager_(heapManager),
          gc_(gc),
          gcTrigger_(gcTrigger),
          gcStats_(gcStats),
          globalObjectStorage_(globalObjectStorage),
          memStats_(memStats)
    {
    }
    ~MemoryManager();

    InternalAllocatorPtr internalAllocator_;
    HeapManager *heapManager_;
    GC *gc_;
    GCTrigger *gcTrigger_;
    GCStats *gcStats_;
    GlobalObjectStorage *globalObjectStorage_;
    MemStatsType *memStats_;

    friend class mem::Allocator;
};

}  // namespace ark::mem

#endif  // PANDA_MEM_MEMORY_MANAGER_H
