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

#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem {

template <class LanguageConfig>
bool GenerationalGC<LanguageConfig>::ShouldRunTenuredGC(const GCTask &task)
{
    return this->IsOnPygoteFork() || task.reason == GCTaskCause::OOM_CAUSE ||
           task.reason == GCTaskCause::EXPLICIT_CAUSE || task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE ||
           task.reason == GCTaskCause::STARTUP_COMPLETE_CAUSE;
}

template <class LanguageConfig>
PandaString GenerationalGC<LanguageConfig>::MemStats::Dump()
{
    PandaStringStream statistic;
    statistic << "Young freed " << youngFreeObjectCount_ << " objects ("
              << helpers::MemoryConverter(youngFreeObjectSize_) << ") Young moved " << youngMoveObjectCount_
              << " objects, " << youngMoveObjectSize_ << " bytes (" << helpers::MemoryConverter(youngMoveObjectSize_)
              << ")";
    if (tenuredFreeObjectSize_ > 0U) {
        statistic << " Tenured freed " << tenuredFreeObjectSize_ << "("
                  << helpers::MemoryConverter(tenuredFreeObjectSize_) << ")";
    }
    return statistic.str();
}

template <class LanguageConfig>
void GenerationalGC<LanguageConfig>::UpdateMemStats(size_t bytesInHeapBefore, bool updateTenuredStats,
                                                    bool recordAllocationForMovedObjects)
{
    size_t youngMoveSize = this->memStats_.GetSizeMovedYoung();
    size_t youngMoveCount = this->memStats_.GetCountMovedYoung();
    size_t youngDeleteSize = this->memStats_.GetSizeFreedYoung();
    size_t youngDeleteCount = this->memStats_.GetCountFreedYoung();
    size_t tenuredMoveSize = updateTenuredStats ? this->memStats_.GetSizeMovedTenured() : 0;
    size_t tenuredMoveCount = updateTenuredStats ? this->memStats_.GetCountMovedTenured() : 0;
    size_t tenuredDeleteSize = updateTenuredStats ? this->memStats_.GetSizeFreedTenured() : 0;
    size_t tenuredDeleteCount = updateTenuredStats ? this->memStats_.GetCountFreedTenured() : 0;

    auto *vmMemStats = this->GetPandaVm()->GetMemStats();
    GCInstanceStats *gcStats = this->GetStats();

    if (recordAllocationForMovedObjects) {
        vmMemStats->RecordAllocateObjects(youngMoveCount + tenuredMoveCount, youngMoveSize + tenuredMoveSize,
                                          SpaceType::SPACE_TYPE_OBJECT);
    }
    if (youngMoveSize > 0) {
        gcStats->AddMemoryValue(youngMoveSize, MemoryTypeStats::MOVED_BYTES);
        gcStats->AddObjectsValue(youngMoveCount, ObjectTypeStats::MOVED_OBJECTS);
        vmMemStats->RecordYoungMovedObjects(youngMoveCount, youngMoveSize, SpaceType::SPACE_TYPE_OBJECT);
    }
    if (youngDeleteSize > 0) {
        gcStats->AddMemoryValue(youngDeleteSize, MemoryTypeStats::YOUNG_FREED_BYTES);
        gcStats->AddObjectsValue(youngDeleteCount, ObjectTypeStats::YOUNG_FREED_OBJECTS);
    }

    if (bytesInHeapBefore > 0) {
        gcStats->AddCopiedRatioValue(static_cast<double>(youngMoveSize + tenuredMoveSize) / bytesInHeapBefore);
    }

    if (tenuredMoveSize > 0) {
        gcStats->AddMemoryValue(tenuredMoveSize, MemoryTypeStats::MOVED_BYTES);
        gcStats->AddObjectsValue(tenuredMoveCount, ObjectTypeStats::MOVED_OBJECTS);
        vmMemStats->RecordTenuredMovedObjects(tenuredMoveCount, tenuredMoveSize, SpaceType::SPACE_TYPE_OBJECT);
    }
    if (tenuredDeleteSize > 0) {
        gcStats->AddMemoryValue(tenuredDeleteSize, MemoryTypeStats::ALL_FREED_BYTES);
        gcStats->AddObjectsValue(tenuredDeleteCount, ObjectTypeStats::ALL_FREED_OBJECTS);
    }
    vmMemStats->RecordFreeObjects(youngDeleteCount + tenuredDeleteCount, youngDeleteSize + tenuredDeleteSize,
                                  SpaceType::SPACE_TYPE_OBJECT);
}

template <class LanguageConfig>
void GenerationalGC<LanguageConfig>::CreateCardTable(InternalAllocatorPtr internalAllocatorPtr, uintptr_t minAddress,
                                                     size_t size)
{
    cardTable_ = MakePandaUnique<CardTable>(internalAllocatorPtr, minAddress, size);
    ASSERT(cardTable_.get() != nullptr);
    cardTable_->Initialize();
}

template <class LanguageConfig>
bool GenerationalGC<LanguageConfig>::Trigger(PandaUniquePtr<GCTask> task)
{
    // Check current heap size.
    // Collect Young gen.
    // If threshold for tenured gen - collect tenured gen.
    // NOTE(dtrubenkov): change for concurrent mode
    return this->AddGCTask(true, std::move(task));
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(GenerationalGC);
}  // namespace ark::mem
