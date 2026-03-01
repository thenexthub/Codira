/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H
#define RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H

#include "runtime/include/mem/allocator.h"
namespace ark::mem {

template <typename AllocT, bool NEED_LOCK>
inline void *ObjectAllocatorBase::AllocateSafe(size_t size, Alignment align, AllocT *objectAllocator, size_t poolSize,
                                               SpaceType spaceType, HeapSpace *heapSpace)
{
    void *mem = objectAllocator->template Alloc<NEED_LOCK>(size, align);
    if (UNLIKELY(mem == nullptr)) {
        return AddPoolsAndAlloc<AllocT, NEED_LOCK>(size, align, objectAllocator, poolSize, spaceType, heapSpace);
    }
    return mem;
}

template <typename AllocT, bool NEED_LOCK>
// CC-OFFNXT(G.FUD.06) perf critical
inline void *ObjectAllocatorBase::AddPoolsAndAlloc(size_t size, Alignment align, AllocT *objectAllocator,
                                                   size_t poolSize, SpaceType spaceType, HeapSpace *heapSpace)
{
    void *mem = nullptr;
    static os::memory::Mutex poolLock;
    os::memory::LockHolder<os::memory::Mutex, NEED_LOCK> lock(poolLock);
    while (true) {
        auto pool = heapSpace->TryAllocPool(poolSize, spaceType, AllocT::GetAllocatorType(), objectAllocator);
        if (UNLIKELY(pool.GetMem() == nullptr)) {
            return nullptr;
        }
        bool addedMemoryPool = objectAllocator->AddMemoryPool(pool.GetMem(), pool.GetSize());
        if (!addedMemoryPool) {
            LOG(FATAL, ALLOC) << "ObjectAllocator: couldn't add memory pool to object allocator";
        }
        mem = objectAllocator->template Alloc<NEED_LOCK>(size, align);
        if (mem != nullptr) {
            break;
        }
    }
    return mem;
}

template <MTModeT MT_MODE>
template <bool NEED_LOCK>
void *ObjectAllocatorGen<MT_MODE>::AllocateTenuredImpl(size_t size)
{
    void *mem = nullptr;
    Alignment align = DEFAULT_ALIGNMENT;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    if (alignedSize <= ObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, ObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe<ObjectAllocator, NEED_LOCK>(size, align, objectAllocator_, poolSize,
                                                       SpaceType::SPACE_TYPE_OBJECT, &heapSpaces_);
    } else if (alignedSize <= LargeObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe<LargeObjectAllocator, NEED_LOCK>(size, align, largeObjectAllocator_, poolSize,
                                                            SpaceType::SPACE_TYPE_OBJECT, &heapSpaces_);
    } else {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
        mem = AllocateSafe<HumongousObjectAllocator, NEED_LOCK>(size, align, humongousObjectAllocator_, poolSize,
                                                                SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, &heapSpaces_);
    }
    return mem;
}

}  // namespace ark::mem

#endif  // RUNTIME_INCLUDE_MEM_ALLOCATOR_INL_H
