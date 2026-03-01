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
// These includes to avoid linker error:

#include "runtime/arch/memory_helpers.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/allocator-inl.h"
#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/mem.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_header.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/bump-allocator-inl.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/tlab.h"

namespace ark::mem {

Allocator::~Allocator() = default;

ObjectAllocatorBase::ObjectAllocatorBase(MemStatsType *memStats, GCCollectMode gcCollectMode,
                                         bool createPygoteSpaceAllocator)
    : Allocator(memStats, AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT, gcCollectMode)
{
    if (createPygoteSpaceAllocator) {
        pygoteSpaceAllocator_ = new (std::nothrow) PygoteAllocator(memStats);
        pygoteAllocEnabled_ = true;
    }
}

ObjectAllocatorBase::~ObjectAllocatorBase()
{
    // NOLINTNEXTLINE(readability-delete-null-pointer)
    if (pygoteSpaceAllocator_ != nullptr) {
        delete pygoteSpaceAllocator_;
        pygoteSpaceAllocator_ = nullptr;
    }
}

bool ObjectAllocatorBase::HaveEnoughPoolsInObjectSpace(size_t poolsNum) const
{
    auto memPool = PoolManager::GetMmapMemPool();
    auto poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, PANDA_DEFAULT_ALLOCATOR_POOL_SIZE);
    return memPool->HaveEnoughPoolsInObjectSpace(poolsNum, poolSize);
}

void ObjectAllocatorBase::MemoryInitialize(void *mem, size_t size) const
{
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    memset_s(mem, size, 0, size);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // zero init should be visible from other threads even if pointer to object was fetched
    // without 'volatile' specifier so full memory barrier is required
    // required by some language-specs
    arch::FullMemoryBarrier();
}

void ObjectAllocatorBase::ObjectMemoryInit(void *mem, size_t size) const
{
    if (mem == nullptr) {
        return;
    }
    [[maybe_unused]] auto *object = static_cast<ObjectHeader *>(mem);
    ASSERT(object->AtomicGetMark().GetValue() == 0);
    ASSERT(object->ClassAddr<BaseClass *>() == nullptr);
    ASSERT(size >= ObjectHeader::ObjectHeaderSize());
    // zeroing according to newobj description in ISA
    size_t sizeToInit = size - ObjectHeader::ObjectHeaderSize();
    void *memToInit = ToVoidPtr(ToUintPtr(mem) + ObjectHeader::ObjectHeaderSize());
    MemoryInitialize(memToInit, sizeToInit);
}

void ObjectAllocatorBase::IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ScopedChangeThreadStatus ets(thread, ThreadStatus::RUNNING);
    ScopedSuspendAllThreadsRunning ssatr(thread->GetVM()->GetRendezvous());
    IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
ObjectAllocatorNoGen<MT_MODE>::ObjectAllocatorNoGen(MemStatsType *memStats, bool createPygoteSpaceAllocator)
    : ObjectAllocatorBase(memStats, GCCollectMode::GC_ALL, createPygoteSpaceAllocator)
{
    const auto &options = Runtime::GetOptions();
    heapSpace_.Initialize(MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                          options.GetMinHeapFreePercentage(), options.GetMaxHeapFreePercentage());
    if (createPygoteSpaceAllocator) {
        ASSERT(pygoteSpaceAllocator_ != nullptr);
        pygoteSpaceAllocator_->SetHeapSpace(&heapSpace_);
    }
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats);
    ASSERT(objectAllocator_ != nullptr);
    largeObjectAllocator_ = new (std::nothrow) LargeObjectAllocator(memStats);
    ASSERT(largeObjectAllocator_ != nullptr);
    humongousObjectAllocator_ = new (std::nothrow) HumongousObjectAllocator(memStats);
    ASSERT(humongousObjectAllocator_ != nullptr);
}

template <MTModeT MT_MODE>
ObjectAllocatorNoGen<MT_MODE>::~ObjectAllocatorNoGen()
{
    delete objectAllocator_;
    delete largeObjectAllocator_;
    delete humongousObjectAllocator_;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorNoGen<MT_MODE>::Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                              ObjMemInitPolicy objInit, [[maybe_unused]] bool pinned)
{
    void *mem = nullptr;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    if (alignedSize <= ObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, ObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe(size, align, objectAllocator_, poolSize, SpaceType::SPACE_TYPE_OBJECT, &heapSpace_);
    } else if (alignedSize <= LargeObjectAllocator::GetMaxSize()) {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
        mem = AllocateSafe(size, align, largeObjectAllocator_, poolSize, SpaceType::SPACE_TYPE_OBJECT, &heapSpace_);
    } else {
        size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
        mem = AllocateSafe(size, align, humongousObjectAllocator_, poolSize, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                           &heapSpace_);
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorNoGen<MT_MODE>::AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                                        ObjMemInitPolicy objInit)
{
    void *mem = nullptr;
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygoteSpaceAllocator_->CanAllocNonMovable(size, align))) {
        mem = pygoteSpaceAllocator_->Alloc(size, align);
    } else {
        // Without generations - no compaction now, so all allocations are non-movable
        mem = Allocate(size, align, thread, objInit, false);
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
Alignment ObjectAllocatorNoGen<MT_MODE>::CalculateAllocatorAlignment(size_t align)
{
    ASSERT(GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
    return GetAlignment(align);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveAllPools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveAllPools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveFreePools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveFreePools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    objectAllocator_->IterateOverObjects(objectVisitor);
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    objectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    objectAllocator_->VisitAndRemoveAllPools(
        [](void *mem, size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    delete objectAllocator_;
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats_);
    ASSERT(objectAllocator_ != nullptr);
}

template <MTModeT MT_MODE>
void ObjectAllocatorNoGen<MT_MODE>::Collect(const GCObjectVisitor &gcObjectVisitor,
                                            [[maybe_unused]] GCCollectMode collectMode)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->Collect(gcObjectVisitor);
    }
    objectAllocator_->Collect(gcObjectVisitor);
    largeObjectAllocator_->Collect(gcObjectVisitor);
    humongousObjectAllocator_->Collect(gcObjectVisitor);
}

// if there is a common base class for these allocators, we could split this func and return the pointer to the
// allocator containing the object
template <MTModeT MT_MODE>
bool ObjectAllocatorNoGen<MT_MODE>::ContainObject(const ObjectHeader *obj) const
{
    if (objectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return true;
    }

    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorNoGen<MT_MODE>::IsLive(const ObjectHeader *obj)
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return pygoteSpaceAllocator_->IsLive(obj);
    }
    if (objectAllocator_->ContainObject(obj)) {
        return objectAllocator_->IsLive(obj);
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return largeObjectAllocator_->IsLive(obj);
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return humongousObjectAllocator_->IsLive(obj);
    }
    return false;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorGen<MT_MODE>::Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                            ObjMemInitPolicy objInit, [[maybe_unused]] bool pinned)
{
    void *mem = nullptr;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    if (LIKELY(alignedSize <= GetYoungAllocMaxSize())) {
        mem = youngGenAllocator_->Alloc(size, align);
    } else {
        mem = AllocateTenured(size);
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorGen<MT_MODE>::AllocateNonMovable(size_t size, Alignment align,
                                                      [[maybe_unused]] ark::ManagedThread *thread,
                                                      ObjMemInitPolicy objInit)
{
    void *mem = nullptr;
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygoteSpaceAllocator_->CanAllocNonMovable(size, align))) {
        mem = pygoteSpaceAllocator_->Alloc(size, align);
    } else {
        size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
        if (alignedSize <= ObjectAllocator::GetMaxSize()) {
            size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, ObjectAllocator::GetMinPoolSize());
            mem = AllocateSafe(size, align, nonMovableObjectAllocator_, poolSize,
                               SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, &heapSpaces_);
        } else if (alignedSize <= LargeObjectAllocator::GetMaxSize()) {
            size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, LargeObjectAllocator::GetMinPoolSize());
            mem = AllocateSafe(size, align, largeNonMovableObjectAllocator_, poolSize,
                               SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, &heapSpaces_);
        } else {
            // We don't need special allocator for this
            // Humongous objects are non-movable
            size_t poolSize = std::max(PANDA_DEFAULT_POOL_SIZE, HumongousObjectAllocator::GetMinPoolSize(size));
            mem = AllocateSafe(size, align, humongousObjectAllocator_, poolSize, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                               &heapSpaces_);
        }
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
Alignment ObjectAllocatorGen<MT_MODE>::CalculateAllocatorAlignment(size_t align)
{
    ASSERT(GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
    return GetAlignment(align);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveAllPools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveAllPools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
    nonMovableObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
    largeNonMovableObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveFreePools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveFreePools(memVisitor);
    largeObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
    nonMovableObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
    largeNonMovableObjectAllocator_->VisitAndRemoveFreePools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateOverYoungObjects(const ObjectVisitor &objectVisitor)
{
    youngGenAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateOverTenuredObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    objectAllocator_->IterateOverObjects(objectVisitor);
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
    nonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
    largeNonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    youngGenAllocator_->IterateOverObjects(objectVisitor);
    objectAllocator_->IterateOverObjects(objectVisitor);
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
    nonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
    largeNonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    objectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    largeObjectAllocator_->IterateOverObjects(objectVisitor);
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
    nonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
    largeNonMovableObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    objectAllocator_->VisitAndRemoveAllPools(
        [](void *mem, size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    delete objectAllocator_;
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats_);
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::Collect(const GCObjectVisitor &gcObjectVisitor, GCCollectMode collectMode)
{
    switch (collectMode) {
        case GCCollectMode::GC_MINOR:
            break;
        case GCCollectMode::GC_ALL:
        case GCCollectMode::GC_MAJOR:
            if (pygoteSpaceAllocator_ != nullptr) {
                pygoteSpaceAllocator_->Collect(gcObjectVisitor);
            }
            objectAllocator_->Collect(gcObjectVisitor);
            largeObjectAllocator_->Collect(gcObjectVisitor);
            humongousObjectAllocator_->Collect(gcObjectVisitor);
            nonMovableObjectAllocator_->Collect(gcObjectVisitor);
            largeNonMovableObjectAllocator_->Collect(gcObjectVisitor);
            break;
        case GCCollectMode::GC_FULL:
            UNREACHABLE();
            break;
        case GC_NONE:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
    }
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetLargeObjectMaxSize()
{
    return LargeObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
TLAB *ObjectAllocatorNoGen<MT_MODE>::CreateNewTLAB([[maybe_unused]] size_t tlabSize)
{
    LOG(FATAL, ALLOC) << "TLAB is not supported for this allocator";
    return nullptr;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorNoGen<MT_MODE>::GetTLABMaxAllocSize()
{
    // NOTE(aemelenko): TLAB usage is not supported for non-gen GCs.
    return 0;
}

ObjectAllocatorGenBase::ObjectAllocatorGenBase(MemStatsType *memStats, GCCollectMode gcCollectMode,
                                               bool createPygoteSpaceAllocator)
    : ObjectAllocatorBase(memStats, gcCollectMode, createPygoteSpaceAllocator)
{
    const auto &options = Runtime::GetOptions();
    heapSpaces_.Initialize(options.GetInitYoungSpaceSize(), options.WasSetInitYoungSpaceSize(),
                           options.GetYoungSpaceSize(), options.WasSetYoungSpaceSize(),
                           MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                           options.GetMinHeapFreePercentage(), options.GetMaxHeapFreePercentage());
    if (createPygoteSpaceAllocator) {
        ASSERT(pygoteSpaceAllocator_ != nullptr);
        pygoteSpaceAllocator_->SetHeapSpace(&heapSpaces_);
    }
}

template <MTModeT MT_MODE>
ObjectAllocatorGen<MT_MODE>::ObjectAllocatorGen(MemStatsType *memStats, bool createPygoteSpaceAllocator)
    : ObjectAllocatorGenBase(memStats, GCCollectMode::GC_ALL, createPygoteSpaceAllocator)
{
    // For Gen-GC we use alone pool for young space, so we will use full such pool
    heapSpaces_.UseFullYoungSpace();
    size_t youngSpaceSize = heapSpaces_.GetCurrentYoungSize();
    size_t initTlabSize = Runtime::GetOptions().GetInitTlabSize();
    auto youngSharedSpaceSize = Runtime::GetOptions().GetYoungSharedSpaceSize();
    ASSERT(youngSpaceSize >= youngSharedSpaceSize);
    ASSERT(initTlabSize != 0);
    size_t maxTlabsCountInYoungGen;
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        // For single-threaded VMs allocate whole private young space for TLAB
        maxTlabsCountInYoungGen = 1;
    } else {
        maxTlabsCountInYoungGen = (youngSpaceSize - youngSharedSpaceSize) / initTlabSize;
        ASSERT(((youngSpaceSize - youngSharedSpaceSize) % initTlabSize) == 0);
    }
    ASSERT(maxTlabsCountInYoungGen * initTlabSize <= youngSpaceSize);

    // NOTE(aemelenko): Missed an allocator pointer
    // because we construct BumpPointer Allocator after calling AllocArena method
    auto youngPool = heapSpaces_.AllocAlonePoolForYoung(SpaceType::SPACE_TYPE_OBJECT,
                                                        YoungGenAllocator::GetAllocatorType(), &youngGenAllocator_);
    youngGenAllocator_ = new (std::nothrow)
        YoungGenAllocator(std::move(youngPool), SpaceType::SPACE_TYPE_OBJECT, memStats, maxTlabsCountInYoungGen);
    objectAllocator_ = new (std::nothrow) ObjectAllocator(memStats);
    largeObjectAllocator_ = new (std::nothrow) LargeObjectAllocator(memStats);
    humongousObjectAllocator_ = new (std::nothrow) HumongousObjectAllocator(memStats);
    nonMovableObjectAllocator_ = new (std::nothrow) ObjectAllocator(memStats, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    largeNonMovableObjectAllocator_ =
        new (std::nothrow) LargeObjectAllocator(memStats, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    memStats_ = memStats;
    GetYoungRanges().push_back({0, 0});
}

template <MTModeT MT_MODE>
ObjectAllocatorGen<MT_MODE>::~ObjectAllocatorGen()
{
    // need to free the pool space when the allocator destroy
    youngGenAllocator_->VisitAndRemoveAllPools(
        [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    delete youngGenAllocator_;
    delete objectAllocator_;
    delete largeObjectAllocator_;
    delete humongousObjectAllocator_;
    delete nonMovableObjectAllocator_;
    delete largeNonMovableObjectAllocator_;
}
template <MTModeT MT_MODE>
size_t ObjectAllocatorGen<MT_MODE>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorGen<MT_MODE>::GetLargeObjectMaxSize()
{
    return LargeObjectAllocator::GetMaxSize();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::IsObjectInYoungSpace(const ObjectHeader *obj)
{
    if (!youngGenAllocator_) {
        return false;
    }
    return youngGenAllocator_->GetMemRange().IsAddressInRange(ToUintPtr(obj));
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::IsIntersectedWithYoung(const MemRange &memRange)
{
    return youngGenAllocator_->GetMemRange().IsIntersect(memRange);
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::IsObjectInNonMovableSpace(const ObjectHeader *obj)
{
    return nonMovableObjectAllocator_->ContainObject(obj);
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::IsNonMovable(const ObjectHeader *obj)
{
    return objectAllocator_->ContainObject(obj) || nonMovableObjectAllocator_->ContainObject(obj) ||
           largeObjectAllocator_->ContainObject(obj) || humongousObjectAllocator_->ContainObject(obj) ||
           largeNonMovableObjectAllocator_->ContainObject(obj);
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::HasYoungSpace()
{
    return youngGenAllocator_ != nullptr;
}

template <MTModeT MT_MODE>
const std::vector<MemRange> &ObjectAllocatorGen<MT_MODE>::GetYoungSpaceMemRanges()
{
    return GetYoungRanges();
}

template <MTModeT MT_MODE>
std::vector<MarkBitmap *> &ObjectAllocatorGen<MT_MODE>::GetYoungSpaceBitmaps()
{
    static std::vector<MarkBitmap *> ret;
    LOG(FATAL, ALLOC) << "GetYoungSpaceBitmaps not applicable for ObjectAllocatorGen";
    return ret;
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::ResetYoungAllocator()
{
    MemStatsType *memStats = memStats_;
    auto threadCallback = [&memStats](ManagedThread *thread) {
        if (!PANDA_TRACK_TLAB_ALLOCATIONS && (thread->GetTLAB()->GetOccupiedSize() != 0)) {
            memStats->RecordAllocateObject(thread->GetTLAB()->GetOccupiedSize(), SpaceType::SPACE_TYPE_OBJECT);
        }
        if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
            thread->GetWeightedTlabAverage()->ComputeNewSumAndResetSamples();
        }
        // Here we should not collect current TLAB fill statistics for adaptive size
        // since it may not be completely filled before resetting
        thread->ClearTLAB();
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(threadCallback);
    youngGenAllocator_->Reset();
}

template <MTModeT MT_MODE>
TLAB *ObjectAllocatorGen<MT_MODE>::CreateNewTLAB(size_t tlabSize)
{
    TLAB *newTlab = youngGenAllocator_->CreateNewTLAB(tlabSize);
    if (newTlab != nullptr) {
        ASAN_UNPOISON_MEMORY_REGION(newTlab->GetStartAddr(), newTlab->GetSize());
        MemoryInitialize(newTlab->GetStartAddr(), newTlab->GetSize());
        ASAN_POISON_MEMORY_REGION(newTlab->GetStartAddr(), newTlab->GetSize());
    }
    return newTlab;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorGen<MT_MODE>::GetTLABMaxAllocSize()
{
    if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
        return Runtime::GetOptions().GetMaxTlabSize();
    }
    return Runtime::GetOptions().GetInitTlabSize();
}

/* static */
template <MTModeT MT_MODE>
size_t ObjectAllocatorGen<MT_MODE>::GetYoungAllocMaxSize()
{
    if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
        return Runtime::GetOptions().GetMaxTlabSize();
    }
    return Runtime::GetOptions().GetInitTlabSize();
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::IterateOverObjectsInRange(MemRange memRange, const ObjectVisitor &objectVisitor)
{
    // we need ensure that the mem range related to a card must be located in one allocator
    auto spaceType = PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(ToVoidPtr(memRange.GetStartAddress()));
    auto allocInfo = PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(ToVoidPtr(memRange.GetStartAddress()));
    auto *allocator = allocInfo.GetAllocatorHeaderAddr();
    switch (spaceType) {
        case SpaceType::SPACE_TYPE_OBJECT:
            if (allocator == objectAllocator_) {
                objectAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                            ToVoidPtr(memRange.GetEndAddress()));
            } else if (allocator == pygoteSpaceAllocator_) {
                pygoteSpaceAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                                 ToVoidPtr(memRange.GetEndAddress()));
            } else if (allocator == &youngGenAllocator_) {
                youngGenAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                              ToVoidPtr(memRange.GetEndAddress()));
            } else if (allocator == largeObjectAllocator_) {
                largeObjectAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                                 ToVoidPtr(memRange.GetEndAddress()));
            } else {
                // if we reach this line, we may have an issue with multiVM CardTable iteration
                UNREACHABLE();
            }
            break;
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
            if (allocator == humongousObjectAllocator_) {
                humongousObjectAllocator_->IterateOverObjectsInRange(
                    objectVisitor, ToVoidPtr(memRange.GetStartAddress()), ToVoidPtr(memRange.GetEndAddress()));
            } else {
                // if we reach this line, we may have an issue with multiVM CardTable iteration
                UNREACHABLE();
            }
            break;
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
            if (allocator == nonMovableObjectAllocator_) {
                nonMovableObjectAllocator_->IterateOverObjectsInRange(
                    objectVisitor, ToVoidPtr(memRange.GetStartAddress()), ToVoidPtr(memRange.GetEndAddress()));
            } else if (allocator == largeNonMovableObjectAllocator_) {
                largeNonMovableObjectAllocator_->IterateOverObjectsInRange(
                    objectVisitor, ToVoidPtr(memRange.GetStartAddress()), ToVoidPtr(memRange.GetEndAddress()));
            } else {
                // if we reach this line, we may have an issue with multiVM CardTable iteration
                UNREACHABLE();
            }
            break;
        default:
            // if we reach this line, we may have an issue with multiVM CardTable iteration
            UNREACHABLE();
            break;
    }
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::ContainObject(const ObjectHeader *obj) const
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return true;
    }
    if (youngGenAllocator_->ContainObject(obj)) {
        return true;
    }
    if (objectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (nonMovableObjectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (largeNonMovableObjectAllocator_->ContainObject(obj)) {
        return true;
    }

    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorGen<MT_MODE>::IsLive(const ObjectHeader *obj)
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return pygoteSpaceAllocator_->IsLive(obj);
    }
    if (youngGenAllocator_->ContainObject(obj)) {
        return youngGenAllocator_->IsLive(obj);
    }
    if (objectAllocator_->ContainObject(obj)) {
        return objectAllocator_->IsLive(obj);
    }
    if (largeObjectAllocator_->ContainObject(obj)) {
        return largeObjectAllocator_->IsLive(obj);
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return humongousObjectAllocator_->IsLive(obj);
    }
    if (nonMovableObjectAllocator_->ContainObject(obj)) {
        return nonMovableObjectAllocator_->IsLive(obj);
    }
    if (largeNonMovableObjectAllocator_->ContainObject(obj)) {
        return largeNonMovableObjectAllocator_->IsLive(obj);
    }

    return false;
}

template <MTModeT MT_MODE>
void ObjectAllocatorGen<MT_MODE>::UpdateSpaceData()
{
    GetYoungRanges().push_back(youngGenAllocator_->GetMemRange());
}

void ObjectAllocatorGenBase::InvalidateSpaceData()
{
    ranges_.clear();
    youngBitmaps_.clear();
}

template class ObjectAllocatorGen<MT_MODE_SINGLE>;
template class ObjectAllocatorGen<MT_MODE_MULTI>;
template class ObjectAllocatorGen<MT_MODE_TASK>;
template class ObjectAllocatorNoGen<MT_MODE_SINGLE>;
template class ObjectAllocatorNoGen<MT_MODE_MULTI>;
template class ObjectAllocatorNoGen<MT_MODE_TASK>;

}  // namespace ark::mem
