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

#ifndef RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H
#define RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H

#include "runtime/include/mem/allocator.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/region_allocator-inl.h"
#include "runtime/mem/gc/g1/g1-allocator_constants.h"

namespace ark::mem {
class ObjectAllocConfigWithCrossingMap;
class ObjectAllocConfig;
class TLAB;

template <MTModeT MT_MODE = MT_MODE_MULTI>
class ObjectAllocatorG1 final : public ObjectAllocatorGenBase {
    using ObjectAllocator = RegionAllocator<ObjectAllocConfig>;
    using NonMovableAllocator = RegionNonmovableAllocator<ObjectAllocConfig, RegionAllocatorLockConfig::CommonLock,
                                                          FreeListAllocator<ObjectAllocConfig>>;
    using HumongousObjectAllocator =
        RegionHumongousAllocator<ObjectAllocConfig>;  // Allocator used for humongous objects

    // REGION_SIZE should not change here.
    // If it is necessary to change this value, it must be done through changes to G1_REGION_SIZE
    static constexpr size_t REGION_SIZE = mem::G1_REGION_SIZE;
    static_assert(REGION_SIZE == mem::G1_REGION_SIZE);

public:
    using GarbageRegions = ObjectAllocator::GarbageRegions;

    NO_MOVE_SEMANTIC(ObjectAllocatorG1);
    NO_COPY_SEMANTIC(ObjectAllocatorG1);

    explicit ObjectAllocatorG1(MemStatsType *memStats, bool createPygoteSpaceAllocator);

    ~ObjectAllocatorG1() final = default;

    void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread, ObjMemInitPolicy objInit,
                   bool pinned) final;

    void *AllocateNonMovable(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                             ObjMemInitPolicy objInit) final;

    void PinObject(ObjectHeader *object) final;

    void UnpinObject(ObjectHeader *object) final;

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor) final;

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor) final;

    void IterateOverYoungObjects(const ObjectVisitor &objectVisitor) final;

    size_t GetMaxYoungRegionsCount();

    PandaVector<Region *> GetYoungRegions();

    PandaVector<Region *> GetMovableRegions();

    PandaVector<Region *> GetNonYoungRegions() const;

    PandaVector<Region *> GetAllRegions();

    /// Returns a vector which contains non-movable and humongous regions
    PandaVector<Region *> GetNonRegularRegions();

    void IterateOverTenuredObjects(const ObjectVisitor &objectVisitor) final;

    void IterateOverHumongousObjects(const ObjectVisitor &objectVisitor);

    void IterateOverObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates all objects in object allocator
    void IterateRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    /// @brief iterates objects in all allocators except object allocator
    void IterateNonRegularSizeObjects(const ObjectVisitor &objectVisitor) final;

    void FreeObjectsMovedToPygoteSpace() final;

    void Collect(const GCObjectVisitor &gcObjectVisitor, GCCollectMode collectMode) final
    {
        (void)gcObjectVisitor;
        (void)collectMode;
        UNREACHABLE();
    }

    /**
     * Collect non regular regions (i.e. remove dead objects from Humongous and NonMovable regions
     * and remove empty regions).
     */
    void CollectNonRegularRegions(const RegionsVisitor &regionVisitor, const GCObjectVisitor &gcObjectVisitor);

    size_t GetRegularObjectMaxSize() final;

    size_t GetLargeObjectMaxSize() final;

    bool IsObjectInYoungSpace(const ObjectHeader *obj) final;

    bool IsIntersectedWithYoung(const MemRange &memRange) final;

    bool HasYoungSpace() final;

    const std::vector<MemRange> &GetYoungSpaceMemRanges() final;

    template <bool INCLUDE_CURRENT_REGION>
    void GetTopGarbageRegions(double garbageThreshold, GarbageRegions &garbageRegions,
                              PandaVector<Region *> &emptyRegions)
    {
        return objectAllocator_->template GetTopGarbageRegions<INCLUDE_CURRENT_REGION>(garbageThreshold, garbageRegions,
                                                                                       emptyRegions);
    }

    std::vector<MarkBitmap *> &GetYoungSpaceBitmaps() final;

    void ReserveRegionIfNeeded()
    {
        objectAllocator_->ReserveRegionIfNeeded();
    }

    void ReleaseReservedRegion()
    {
        objectAllocator_->ReleaseReservedRegion();
    }

    void ResetYoungAllocator() final;

    template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
              OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
    void ResetRegions(const Container &regions)
    {
        objectAllocator_->ResetSeveralSpecificRegions<REGIONS_TYPE, REGIONS_RELEASE_POLICY, OS_PAGES_POLICY, NEED_LOCK>(
            regions);
    }

    TLAB *CreateNewTLAB(size_t tlabSize) final;

    /**
     * @brief This method should be used carefully, since in case of adaptive TLAB
     * it only shows max possible size (grow limit) of a TLAB
     */
    size_t GetTLABMaxAllocSize() final;

    bool IsTLABSupported() final
    {
        return true;
    }

    void IterateOverObjectsInRange(MemRange memRange, const ObjectVisitor &objectVisitor) final;

    bool ContainObject(const ObjectHeader *obj) const final;

    bool IsLive(const ObjectHeader *obj) final;

    size_t VerifyAllocatorStatus() final
    {
        LOG(FATAL, ALLOC) << "Not implemented";
        return 0;
    }

    [[nodiscard]] void *AllocateLocal([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align,
                                      [[maybe_unused]] ark::ManagedThread *thread) final
    {
        LOG(FATAL, ALLOC) << "ObjectAllocatorGen: AllocateLocal not supported";
        return nullptr;
    }

    bool IsObjectInNonMovableSpace(const ObjectHeader *obj) final;
    bool IsNonMovable(const ObjectHeader *obj) final;

    void UpdateSpaceData() final;

    void CompactYoungRegions(const GCObjectVisitor &deathChecker, const ObjectVisitorEx &moveChecker);

    void AddPromotedRegionToQueueIfPinned(Region *region)
    {
        objectAllocator_->AddPromotedRegionToQueueIfPinned(region);
    }
    template <RegionFlag REGION_TYPE, bool USE_MARKBITMAP = false>
    void CompactRegion(Region *region, const GCObjectVisitor &deathChecker, const ObjectVisitorEx &moveChecker)
    {
        objectAllocator_->template CompactSpecificRegion<REGION_TYPE, RegionFlag::IS_OLD, USE_MARKBITMAP>(
            region, deathChecker, moveChecker);
    }

    template <bool USE_MARKBITMAP, bool FULL_GC>
    size_t PromoteYoungRegion(Region *region, const GCObjectVisitor &deathChecker,
                              const ObjectVisitor &promotionChecker)
    {
        ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
        return objectAllocator_->template PromoteYoungRegion<USE_MARKBITMAP, FULL_GC>(region, deathChecker,
                                                                                      promotionChecker);
    }

    void CompactTenuredRegions(const PandaVector<Region *> &regions, const GCObjectVisitor &deathChecker,
                               const ObjectVisitorEx &moveChecker);

    template <bool USE_ATOMIC = true>
    Region *PopFromOldRegionQueue()
    {
        return objectAllocator_->template PopFromRegionQueue<USE_ATOMIC, RegionFlag::IS_OLD>();
    }

    template <bool USE_ATOMIC = true>
    void PushToOldRegionQueue(Region *region)
    {
        objectAllocator_->template PushToRegionQueue<USE_ATOMIC, RegionFlag::IS_OLD>(region);
    }

    template <bool USE_ATOMIC = true>
    Region *CreateAndSetUpNewOldRegion()
    {
        return objectAllocator_->template CreateAndSetUpNewRegionWithLock<USE_ATOMIC, RegionFlag::IS_OLD>();
    }

    void ClearCurrentTenuredRegion()
    {
        objectAllocator_->template ClearCurrentRegion<IS_OLD>();
    }

    static constexpr size_t GetRegionSize()
    {
        return REGION_SIZE;
    }

    bool HaveTenuredSize(size_t numRegions) const
    {
        return objectAllocator_->GetSpace()->GetPool()->HaveTenuredSize(numRegions * ObjectAllocator::REGION_SIZE);
    }

    bool HaveFreeRegions(size_t numRegions) const
    {
        return objectAllocator_->GetSpace()->GetPool()->HaveFreeRegions(numRegions, ObjectAllocator::REGION_SIZE);
    }

    static constexpr size_t GetYoungAllocMaxSize()
    {
        // NOTE(dtrubenkov): FIX to more meaningful value
        return ObjectAllocator::GetMaxRegularObjectSize();
    }

    template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
    void ReleaseEmptyRegions()
    {
        objectAllocator_->ReleaseEmptyRegions<REGION_TYPE, OS_PAGES_POLICY>();
    }

    void SetDesiredEdenLength(size_t edenLength)
    {
        objectAllocator_->SetDesiredEdenLength(edenLength);
    }

    double CalculateNonMovableExternalFragmentation()
    {
        return nonmovableAllocator_->CalculateExternalFragmentation();
    }

    double CalculateInternalOldFragmentation()
    {
        return objectAllocator_->CalculateInternalOldFragmentation();
    }

    double CalculateInternalHumongousFragmentation()
    {
        return humongousObjectAllocator_->CalculateInternalFragmentation();
    }

    double CalculateOldDeadObjectsRatio()
    {
        return objectAllocator_->CalculateDeadObjectsRatio();
    }

    double CalculateNonMovableDeadObjectsRatio()
    {
        return nonmovableAllocator_->CalculateDeadObjectsRatio();
    }

private:
    Alignment CalculateAllocatorAlignment(size_t align) final;

    PandaUniquePtr<ObjectAllocator> objectAllocator_ {nullptr};
    PandaUniquePtr<NonMovableAllocator> nonmovableAllocator_ {nullptr};
    PandaUniquePtr<HumongousObjectAllocator> humongousObjectAllocator_ {nullptr};
    MemStatsType *memStats_ {nullptr};

    void *AllocateTenured(size_t size) final;

    void *AllocateTenuredWithoutLocks(size_t size) final;

    friend class AllocTypeConfigG1;
};

}  // namespace ark::mem

#endif  // RUNTIME_MEM_GC_G1_G1_ALLOCATOR_H
