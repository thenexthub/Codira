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

#include "common_interfaces/heap/heap_allocator.h"

#include "common_components/heap/heap_allocator-inl.h"
#include "common_components/common/type_def.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/allocator/regional_heap.h"

namespace common {
Address AllocateYoungInAllocBuffer(uintptr_t buffer, size_t size)
{
    CHECKF(buffer != 0);
    AllocationBuffer *allocBuffer = reinterpret_cast<AllocationBuffer *>(buffer);
    return allocBuffer->FastAllocateInTlab<AllocBufferType::YOUNG>(size);
}

Address AllocateOldInAllocBuffer(uintptr_t buffer, size_t size)
{
    CHECKF(buffer != 0);
    AllocationBuffer *allocBuffer = reinterpret_cast<AllocationBuffer *>(buffer);
    return allocBuffer->FastAllocateInTlab<common::AllocBufferType::OLD>(size);
}

Address HeapAllocator::AllocateInYoungOrHuge(size_t size, LanguageType language)
{
    auto address = HeapManager::Allocate(size);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

Address HeapAllocator::AllocateInNonmoveOrHuge(size_t size, LanguageType language)
{
    auto address = HeapManager::Allocate(size, AllocType::NONMOVABLE_OBJECT);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

Address HeapAllocator::AllocateInOldOrHuge(size_t size, LanguageType language)
{
    auto address = HeapManager::Allocate(size, AllocType::MOVEABLE_OLD_OBJECT);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

Address HeapAllocator::AllocateInHuge(size_t size, LanguageType language)
{
    auto address = HeapManager::Allocate(size);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

Address HeapAllocator::AllocateInReadOnly(size_t size, LanguageType language)
{
    auto address = HeapManager::Allocate(size, AllocType::READ_ONLY_OBJECT);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

uintptr_t HeapAllocator::AllocateLargeJitFortRegion(size_t size, LanguageType language)
{
    RegionalHeap& allocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    auto address =  allocator.AllocJitFortRegion(size);
    BaseObject::Cast(address)->SetLanguageType(language);
    return address;
}

// below are interfaces used for serialize
Address HeapAllocator::AllocateNoGC(size_t size)
{
    return HeapManager::Allocate(size, AllocType::MOVEABLE_OBJECT, false);
}

Address HeapAllocator::AllocateOldOrLargeNoGC(size_t size)
{
    if (size >= RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD) {
        return AllocateLargeRegion(size);
    }
    return HeapManager::Allocate(size, AllocType::MOVEABLE_OLD_OBJECT, false);
}

Address HeapAllocator::AllocateNonmoveNoGC(size_t size)
{
    return HeapManager::Allocate(size, AllocType::NONMOVABLE_OBJECT, false);
}

Address HeapAllocator::AllocateOldRegion()
{
    RegionalHeap& allocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    return allocator.AllocOldRegion();
}

Address HeapAllocator::AllocateNonMovableRegion()
{
    RegionalHeap& allocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    return allocator.AllocateNonMovableRegion();
}

Address HeapAllocator::AllocateLargeRegion(size_t size)
{
    RegionalHeap& allocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    return allocator.AllocLargeRegion(size);
}

}  // namespace common
