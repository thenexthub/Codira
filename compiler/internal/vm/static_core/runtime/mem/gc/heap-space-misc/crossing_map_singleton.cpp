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

#include "runtime/mem/gc/heap-space-misc/crossing_map_singleton.h"

#include "libarkbase/utils/logger.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map.h"

namespace ark::mem {

CrossingMap *CrossingMapSingleton ::instance_ = nullptr;
os::memory::Mutex CrossingMapSingleton::mutex_;  // NOLINT(fuchsia-statically-constructed-objects)

bool CrossingMapSingleton::Create()
{
    {
        os::memory::LockHolder lock(mutex_);

        InternalAllocatorPtr allocator {InternalAllocator<>::GetInternalAllocatorFromRuntime()};
        if (instance_ != nullptr) {
            LOG(FATAL, RUNTIME) << "CrossingMap is created already";
            return false;
        }
        instance_ = allocator->New<CrossingMap>(allocator, PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                                                PoolManager::GetMmapMemPool()->GetTotalObjectSize());
        instance_->Initialize();
    }
    return true;
}

/* static */
bool CrossingMapSingleton::IsCreated()
{
    return instance_ != nullptr;
}

/* static */
bool CrossingMapSingleton::Destroy()
{
    CrossingMap *tempInstance;
    {
        os::memory::LockHolder lock(mutex_);

        if (instance_ == nullptr) {
            return false;
        }
        tempInstance = instance_;
        instance_ = nullptr;
    }
    tempInstance->Destroy();
    InternalAllocatorPtr allocator {InternalAllocator<>::GetInternalAllocatorFromRuntime()};
    allocator->Delete(tempInstance);
    return true;
}

/* static */
void CrossingMapSingleton::AddObject(void *objAddr, size_t objSize)
{
    GetCrossingMap()->AddObject(objAddr, objSize);
}

/* static */
void CrossingMapSingleton::RemoveObject(void *objAddr, size_t objSize, void *nextObjAddr, void *prevObjAddr,
                                        size_t prevObjSize)
{
    GetCrossingMap()->RemoveObject(objAddr, objSize, nextObjAddr, prevObjAddr, prevObjSize);
}

/* static */
void *CrossingMapSingleton::FindFirstObject(void *startAddr, void *endAddr)
{
    return GetCrossingMap()->FindFirstObject(startAddr, endAddr);
}

/* static */
void CrossingMapSingleton::InitializeCrossingMapForMemory(void *startAddr, size_t size)
{
    return GetCrossingMap()->InitializeCrossingMapForMemory(startAddr, size);
}

/* static */
void CrossingMapSingleton::RemoveCrossingMapForMemory(void *startAddr, size_t size)
{
    return GetCrossingMap()->RemoveCrossingMapForMemory(startAddr, size);
}

/* static */
size_t CrossingMapSingleton::GetCrossingMapGranularity()
{
    return PANDA_CROSSING_MAP_GRANULARITY;
}

/* static */
void CrossingMapSingleton::MarkCardsAsYoung(const MemRange &memRange)
{
    auto cardTable = Thread::GetCurrent()->GetVM()->GetGC()->GetCardTable();
    cardTable->MarkCardsAsYoung(memRange);
}

}  // namespace ark::mem
