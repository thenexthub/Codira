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

#include "memory_manager.h"
#include "runtime/include/runtime_options.h"
#include "runtime/mem/refstorage/global_object_storage.h"

#include <runtime/mem/gc/gc.h>
#include <runtime/mem/gc/gc_trigger.h>
#include <runtime/mem/gc/gc_stats.h>
#include <runtime/mem/heap_manager.h>

namespace ark::mem {

static HeapManager *CreateHeapManager(InternalAllocatorPtr internalAllocator, const MemoryManager::HeapOptions &options,
                                      GCType gcType, MemStatsType *memStats)
{
    auto *heapManager = new HeapManager();
    if (heapManager == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to allocate HeapManager";
        return nullptr;
    }

    if (!heapManager->Initialize(gcType, options.multithreadingMode, options.isUseTlabForAllocations, memStats,
                                 internalAllocator, options.isStartAsZygote)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize HeapManager";
        delete heapManager;
        return nullptr;
    }
    heapManager->SetIsFinalizableFunc(options.isObjectFinalizebleFunc);
    heapManager->SetRegisterFinalizeReferenceFunc(options.registerFinalizeReferenceFunc);

    return heapManager;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic, the fix will degrade the readability and maintenance of the code
MemoryManager *MemoryManager::Create(const LanguageContext &ctx, InternalAllocatorPtr internalAllocator, GCType gcType,
                                     const GCSettings &gcSettings, const GCTriggerConfig &gcTriggerConfig,
                                     const HeapOptions &heapOptions)
{
    std::unique_ptr<MemStatsType> memStats = std::make_unique<MemStatsType>();

    HeapManager *heapManager = CreateHeapManager(internalAllocator, heapOptions, gcType, memStats.get());
    if (heapManager == nullptr) {
        return nullptr;
    }

    InternalAllocatorPtr allocator = heapManager->GetInternalAllocator();
    GCStats *gcStats = allocator->New<GCStats>(memStats.get(), gcType, allocator);
    GC *gc = ctx.CreateGC(gcType, heapManager->GetObjectAllocator().AsObjectAllocator(), gcSettings);
    GCTrigger *gcTrigger =
        CreateGCTrigger(memStats.get(), heapManager->GetObjectAllocator().AsObjectAllocator()->GetHeapSpace(),
                        gcTriggerConfig, allocator);
    if (gcSettings.G1EnablePauseTimeGoal() &&
        (gcType != GCType::G1_GC || gcTrigger->GetType() != GCTriggerType::PAUSE_TIME_GOAL_TRIGGER)) {
        LOG(FATAL, RUNTIME) << "Pause time goal is supported with G1 GC and pause-time-goal trigger only";
        return nullptr;
    }

    GlobalObjectStorage *globalObjectStorage = internalAllocator->New<GlobalObjectStorage>(
        internalAllocator, heapOptions.maxGlobalRefSize, heapOptions.isGlobalReferenceSizeCheckEnabled);
    if (globalObjectStorage == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to allocate GlobalObjectStorage";
        return nullptr;
    }

    return new MemoryManager(internalAllocator, heapManager, gc, gcTrigger, gcStats, memStats.release(),
                             globalObjectStorage);
}

/* static */
void MemoryManager::Destroy(MemoryManager *mm)
{
    delete mm;
}

MemoryManager::~MemoryManager()
{
    heapManager_->GetInternalAllocator()->Delete(gc_);
    heapManager_->GetInternalAllocator()->Delete(gcTrigger_);
    heapManager_->GetInternalAllocator()->Delete(gcStats_);
    heapManager_->GetInternalAllocator()->Delete(globalObjectStorage_);

    delete heapManager_;

    // One more check that we don't have memory leak in internal allocator.
    ASSERT(memStats_->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL) == 0);
    delete memStats_;
}

void MemoryManager::Finalize()
{
    heapManager_->Finalize();
}

void MemoryManager::InitializeGC(PandaVM *vm)
{
    heapManager_->SetPandaVM(vm);
    gc_->Initialize(vm);
    gc_->AddListener(gcTrigger_);
}

void MemoryManager::PreStartup()
{
    gc_->PreStartup();
}

void MemoryManager::PreZygoteFork()
{
    gc_->PreZygoteFork();
    heapManager_->PreZygoteFork();
}

void MemoryManager::PostZygoteFork()
{
    gc_->PostZygoteFork();
}

void MemoryManager::StartGC()
{
    gc_->StartGC();
}

void MemoryManager::StopGC()
{
    gc_->StopGC();
}

}  // namespace ark::mem
