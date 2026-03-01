/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/mem/allocator-inl.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/humongous_obj_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem {

template <MTModeT MT_MODE>
ObjectAllocatorG1<MT_MODE>::ObjectAllocatorG1(MemStatsType *memStats, [[maybe_unused]] bool createPygoteSpaceAllocator)
    : ObjectAllocatorGenBase(memStats, GCCollectMode::GC_ALL, false)
{
    size_t reservedTenuredRegionsCount = Runtime::GetOptions().GetG1NumberOfTenuredRegionsAtMixedCollection();
    objectAllocator_ = MakePandaUnique<ObjectAllocator>(memStats, &heapSpaces_, SpaceType::SPACE_TYPE_OBJECT, 0, true,
                                                        reservedTenuredRegionsCount);
    nonmovableAllocator_ =
        MakePandaUnique<NonMovableAllocator>(memStats, &heapSpaces_, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    humongousObjectAllocator_ =
        MakePandaUnique<HumongousObjectAllocator>(memStats, &heapSpaces_, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    memStats_ = memStats;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetRegularObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetLargeObjectMaxSize()
{
    return ObjectAllocator::GetMaxRegularObjectSize();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsObjectInYoungSpace(const ObjectHeader *obj)
{
    Region *regWithObj = ObjectToRegion(obj);
    ASSERT(regWithObj != nullptr);
    return regWithObj->IsYoung();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsIntersectedWithYoung(const MemRange &memRange)
{
    auto youngMemRanges = GetYoungSpaceMemRanges();
    for (const auto &youngMemRange : youngMemRanges) {
        if (youngMemRange.IsIntersect(memRange)) {
            return true;
        }
    }
    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::HasYoungSpace()
{
    return true;
}

template <MTModeT MT_MODE>
const std::vector<MemRange> &ObjectAllocatorG1<MT_MODE>::GetYoungSpaceMemRanges()
{
    return GetYoungRanges();
}

template <MTModeT MT_MODE>
std::vector<MarkBitmap *> &ObjectAllocatorG1<MT_MODE>::GetYoungSpaceBitmaps()
{
    return GetYoungBitmaps();
}

template <MTModeT MT_MODE>
TLAB *ObjectAllocatorG1<MT_MODE>::CreateNewTLAB([[maybe_unused]] size_t tlabSize)
{
    TLAB *newTlab = nullptr;
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        // For single-threaded VMs allocate a whole region for TLAB
        newTlab = objectAllocator_->CreateRegionSizeTLAB();
    } else {
        newTlab = objectAllocator_->CreateTLAB(tlabSize);
    }
    if (newTlab != nullptr && !newTlab->IsZeroed()) {
        ASAN_UNPOISON_MEMORY_REGION(newTlab->GetStartAddr(), newTlab->GetSize());
        MemoryInitialize(newTlab->GetStartAddr(), newTlab->GetSize());
        ASAN_POISON_MEMORY_REGION(newTlab->GetStartAddr(), newTlab->GetSize());
    }
    return newTlab;
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetTLABMaxAllocSize()
{
    if constexpr (MT_MODE == MT_MODE_SINGLE) {
        // For single-threaded VMs we can allocate objects of size up to region size in TLABs.
        return GetYoungAllocMaxSize();
    } else {
        if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
            return Runtime::GetOptions().GetMaxTlabSize();
        }
        return Runtime::GetOptions().GetInitTlabSize();
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverObjectsInRange(MemRange memRange, const ObjectVisitor &objectVisitor)
{
    // we need ensure that the mem range related to a card must be located in one allocator
    auto spaceType = PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(ToVoidPtr(memRange.GetStartAddress()));
    switch (spaceType) {
        case SpaceType::SPACE_TYPE_OBJECT:
            objectAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                        ToVoidPtr(memRange.GetEndAddress()));
            break;
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT: {
            if (pygoteSpaceAllocator_ != nullptr) {
                pygoteSpaceAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                                 ToVoidPtr(memRange.GetEndAddress()));
            }
            auto region = AddrToRegion(ToVoidPtr(memRange.GetStartAddress()));
            region->GetLiveBitmap()->IterateOverMarkedChunkInRange(
                ToVoidPtr(memRange.GetStartAddress()), ToVoidPtr(memRange.GetEndAddress()),
                [&objectVisitor](void *mem) { objectVisitor(reinterpret_cast<ObjectHeader *>(mem)); });
            break;
        }
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
            humongousObjectAllocator_->IterateOverObjectsInRange(objectVisitor, ToVoidPtr(memRange.GetStartAddress()),
                                                                 ToVoidPtr(memRange.GetEndAddress()));
            break;
        default:
            // if we reach this line, we may have an issue with multiVM CardTable iteration
            UNREACHABLE();
            break;
    }
}

// maybe ObjectAllocatorGen and ObjectAllocatorNoGen should have inheritance relationship
template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::ContainObject(const ObjectHeader *obj) const
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return true;
    }
    if (objectAllocator_->ContainObject(obj)) {
        return true;
    }
    if (nonmovableAllocator_->ContainObject(obj)) {
        return true;
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return true;
    }

    return false;
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsLive(const ObjectHeader *obj)
{
    if (pygoteSpaceAllocator_ != nullptr && pygoteSpaceAllocator_->ContainObject(obj)) {
        return pygoteSpaceAllocator_->IsLive(obj);
    }
    if (objectAllocator_->ContainObject(obj)) {
        return objectAllocator_->IsLive(obj);
    }
    if (nonmovableAllocator_->ContainObject(obj)) {
        return nonmovableAllocator_->IsLive(obj);
    }
    if (humongousObjectAllocator_->ContainObject(obj)) {
        return humongousObjectAllocator_->IsLive(obj);
    }
    return false;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                           ObjMemInitPolicy objInit, [[maybe_unused]] bool pinned)
{
    void *mem = nullptr;
    size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
    if (LIKELY(alignedSize <= GetYoungAllocMaxSize())) {
        RegionMem regionMem = objectAllocator_->AllocExt(size, align, pinned);
        if (objInit == ObjMemInitPolicy::REQUIRE_INIT && !regionMem.isZeroed) {
            ObjectMemoryInit(regionMem.mem, size);
        }
        mem = regionMem.mem;
    } else {
        mem = humongousObjectAllocator_->Alloc(size, DEFAULT_ALIGNMENT);
        // Humongous allocations have initialized memory by a default
    }
    return mem;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateNonMovable(size_t size, Alignment align,
                                                     [[maybe_unused]] ark::ManagedThread *thread,
                                                     ObjMemInitPolicy objInit)
{
    void *mem = nullptr;
    // before pygote fork, allocate small non-movable objects in pygote space
    if (UNLIKELY(IsPygoteAllocEnabled() && pygoteSpaceAllocator_->CanAllocNonMovable(size, align))) {
        mem = pygoteSpaceAllocator_->Alloc(size, align);
    } else {
        size_t alignedSize = AlignUp(size, GetAlignmentInBytes(align));
        if (alignedSize <= ObjectAllocator::GetMaxRegularObjectSize()) {
            // NOTE(dtrubenkov): check if we don't need to handle OOM
            mem = nonmovableAllocator_->Alloc(alignedSize, align);
        } else {
            // Humongous objects are non-movable
            mem = humongousObjectAllocator_->Alloc(alignedSize, align);
            // Humongous allocations have initialized memory by a default
            return mem;
        }
    }
    if (objInit == ObjMemInitPolicy::REQUIRE_INIT) {
        ObjectMemoryInit(mem, size);
    }
    return mem;
}

template <MTModeT MT_MODE>
Alignment ObjectAllocatorG1<MT_MODE>::CalculateAllocatorAlignment(size_t align)
{
    ASSERT(GetPurpose() == AllocatorPurpose::ALLOCATOR_PURPOSE_OBJECT);
    return GetAlignment(align);
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateTenured([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MT_MODE>
void *ObjectAllocatorG1<MT_MODE>::AllocateTenuredWithoutLocks([[maybe_unused]] size_t size)
{
    UNREACHABLE();
    return nullptr;
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveAllPools(memVisitor);
    }
    objectAllocator_->VisitAndRemoveAllPools(memVisitor);
    nonmovableAllocator_->VisitAndRemoveAllPools(memVisitor);
    humongousObjectAllocator_->VisitAndRemoveAllPools(memVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->VisitAndRemoveFreePools(memVisitor);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverYoungObjects(const ObjectVisitor &objectVisitor)
{
    auto youngRegions = objectAllocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
    for (auto r : youngRegions) {
        r->template IterateOverObjects(objectVisitor);
    }
}

template <MTModeT MT_MODE>
size_t ObjectAllocatorG1<MT_MODE>::GetMaxYoungRegionsCount()
{
    return GetHeapSpace()->GetMaxYoungSize() / REGION_SIZE;
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetYoungRegions()
{
    return objectAllocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetMovableRegions()
{
    return objectAllocator_->GetAllRegions();
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetNonYoungRegions() const
{
    PandaVector<Region *> regions = objectAllocator_->GetAllSpecificRegions<RegionFlag::IS_OLD>();
    PandaVector<Region *> nonMovableRegions = nonmovableAllocator_->GetAllRegions();
    PandaVector<Region *> humongousRegions = humongousObjectAllocator_->GetAllRegions();
    regions.insert(regions.end(), nonMovableRegions.begin(), nonMovableRegions.end());
    regions.insert(regions.end(), humongousRegions.begin(), humongousRegions.end());
    return regions;
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetAllRegions()
{
    PandaVector<Region *> regions = objectAllocator_->GetAllRegions();
    PandaVector<Region *> nonMovableRegions = nonmovableAllocator_->GetAllRegions();
    PandaVector<Region *> humongousRegions = humongousObjectAllocator_->GetAllRegions();
    regions.insert(regions.end(), nonMovableRegions.begin(), nonMovableRegions.end());
    regions.insert(regions.end(), humongousRegions.begin(), humongousRegions.end());
    return regions;
}

template <MTModeT MT_MODE>
PandaVector<Region *> ObjectAllocatorG1<MT_MODE>::GetNonRegularRegions()
{
    PandaVector<Region *> regions = nonmovableAllocator_->GetAllRegions();
    PandaVector<Region *> humongousRegions = humongousObjectAllocator_->GetAllRegions();
    regions.insert(regions.end(), humongousRegions.begin(), humongousRegions.end());
    return regions;
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CollectNonRegularRegions(const RegionsVisitor &regionVisitor,
                                                          const GCObjectVisitor &gcObjectVisitor)
{
    nonmovableAllocator_->Collect(gcObjectVisitor);
    nonmovableAllocator_->VisitAndRemoveFreeRegions(regionVisitor);
    humongousObjectAllocator_->CollectAndRemoveFreeRegions(regionVisitor, gcObjectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverTenuredObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    objectAllocator_->IterateOverObjects(objectVisitor);
    nonmovableAllocator_->IterateOverObjects(objectVisitor);
    IterateOverHumongousObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverHumongousObjects(const ObjectVisitor &objectVisitor)
{
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

static inline void IterateOverObjectsInRegion(Region *region, const ObjectVisitor &objectVisitor)
{
    if (region->GetLiveBitmap() != nullptr) {
        region->GetLiveBitmap()->IterateOverMarkedChunks(
            [&objectVisitor](void *mem) { objectVisitor(static_cast<ObjectHeader *>(mem)); });
    } else {
        region->IterateOverObjects(objectVisitor);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    for (Region *region : objectAllocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, objectVisitor);
    }
    for (Region *region : nonmovableAllocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, objectVisitor);
    }
    for (Region *region : humongousObjectAllocator_->GetAllRegions()) {
        IterateOverObjectsInRegion(region, objectVisitor);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    objectAllocator_->IterateOverObjects(objectVisitor);
    nonmovableAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor)
{
    if (pygoteSpaceAllocator_ != nullptr) {
        pygoteSpaceAllocator_->IterateOverObjects(objectVisitor);
    }
    humongousObjectAllocator_->IterateOverObjects(objectVisitor);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::FreeObjectsMovedToPygoteSpace()
{
    // clear because we have move all objects in it to pygote space
    // NOTE(dtrubenkov): FIX clean object_allocator_
    objectAllocator_.reset(new (std::nothrow) ObjectAllocator(memStats_, &heapSpaces_));
    ASSERT(objectAllocator_.get() != nullptr);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::ResetYoungAllocator()
{
    auto callback = [](ManagedThread *thread) {
        thread->CollectTLABMetrics();
        if (Runtime::GetOptions().IsAdaptiveTlabSize()) {
            thread->GetWeightedTlabAverage()->ComputeNewSumAndResetSamples();
        }
        // Here we should not collect current TLAB fill statistics for adaptive size
        // since it may not be completely filled before resetting
        thread->ClearTLAB();
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback);
    objectAllocator_->ResetAllSpecificRegions<RegionFlag::IS_EDEN>();
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsObjectInNonMovableSpace(const ObjectHeader *obj)
{
    return nonmovableAllocator_->ContainObject(obj);
}

template <MTModeT MT_MODE>
bool ObjectAllocatorG1<MT_MODE>::IsNonMovable(const ObjectHeader *obj)
{
    return humongousObjectAllocator_->ContainObject(obj) || nonmovableAllocator_->ContainObject(obj);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::UpdateSpaceData()
{
    ASSERT(GetYoungRanges().empty());
    ASSERT(GetYoungBitmaps().empty());
    for (auto r : objectAllocator_->template GetAllSpecificRegions<RegionFlag::IS_EDEN>()) {
        GetYoungRanges().emplace_back(r->Begin(), r->End());
        GetYoungBitmaps().push_back(r->GetMarkBitmap());
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CompactYoungRegions(const GCObjectVisitor &deathChecker,
                                                     const ObjectVisitorEx &moveChecker)
{
    objectAllocator_->template CompactAllSpecificRegions<RegionFlag::IS_EDEN, RegionFlag::IS_OLD>(deathChecker,
                                                                                                  moveChecker);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::CompactTenuredRegions(const PandaVector<Region *> &regions,
                                                       const GCObjectVisitor &deathChecker,
                                                       const ObjectVisitorEx &moveChecker)
{
    objectAllocator_->template CompactSeveralSpecificRegions<RegionFlag::IS_OLD, RegionFlag::IS_OLD>(
        regions, deathChecker, moveChecker);
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::PinObject(ObjectHeader *object)
{
    if (objectAllocator_->ContainObject(object)) {
        objectAllocator_->PinObject(object);
    }
}

template <MTModeT MT_MODE>
void ObjectAllocatorG1<MT_MODE>::UnpinObject(ObjectHeader *object)
{
    if (objectAllocator_->ContainObject(object)) {
        objectAllocator_->UnpinObject(object);
    }
}

template class ObjectAllocatorG1<MT_MODE_SINGLE>;
template class ObjectAllocatorG1<MT_MODE_MULTI>;
template class ObjectAllocatorG1<MT_MODE_TASK>;

}  // namespace ark::mem
