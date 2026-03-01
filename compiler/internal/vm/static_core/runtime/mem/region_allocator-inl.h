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
#ifndef PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H

#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/region_allocator.h"
#include "runtime/mem/region_space-inl.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/arch/memory_helpers.h"
#include "libarkbase/utils/utils.h"

namespace ark::mem {

template <typename LockConfigT>
RegionAllocatorBase<LockConfigT>::RegionAllocatorBase(MemStatsType *memStats, GenerationalSpaces *spaces,
                                                      SpaceType spaceType, AllocatorType allocatorType,
                                                      size_t initSpaceSize, bool extend, size_t regionSize,
                                                      size_t emptyTenuredRegionsMaxCount)
    : memStats_(memStats),
      spaceType_(spaceType),
      spaces_(spaces),
      regionPool_(regionSize, extend, spaces,
                  InternalAllocatorPtr(InternalAllocator<>::GetInternalAllocatorFromRuntime())),
      regionSpace_(spaceType, allocatorType, &regionPool_, emptyTenuredRegionsMaxCount),
      initBlock_(0, nullptr)
{
    ASSERT(spaceType_ == SpaceType::SPACE_TYPE_OBJECT || spaceType_ == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT ||
           spaceType_ == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    ASSERT(regionSize != 0);
    initBlock_ = NULLPOOL;
    if (initSpaceSize > 0) {
        ASSERT(initSpaceSize % regionSize == 0);
        initBlock_ = spaces_->AllocSharedPool(initSpaceSize, spaceType, AllocatorType::REGION_ALLOCATOR, this);
        ASSERT(initBlock_.GetMem() != nullptr);
        ASSERT(initBlock_.GetSize() >= initSpaceSize);
        if (initBlock_.GetMem() != nullptr) {
            regionPool_.InitRegionBlock(ToUintPtr(initBlock_.GetMem()), ToUintPtr(initBlock_.GetMem()) + initSpaceSize);
            ASAN_POISON_MEMORY_REGION(initBlock_.GetMem(), initBlock_.GetSize());
        }
    }
}

template <typename LockConfigT>
RegionAllocatorBase<LockConfigT>::RegionAllocatorBase(MemStatsType *memStats, GenerationalSpaces *spaces,
                                                      SpaceType spaceType, AllocatorType allocatorType,
                                                      RegionPool *sharedRegionPool, size_t emptyTenuredRegionsMaxCount)
    : memStats_(memStats),
      spaces_(spaces),
      spaceType_(spaceType),
      regionPool_(0, false, spaces, nullptr),  // unused
      regionSpace_(spaceType, allocatorType, sharedRegionPool, emptyTenuredRegionsMaxCount),
      initBlock_(0, nullptr)  // unused
{
    ASSERT(spaceType_ == SpaceType::SPACE_TYPE_OBJECT || spaceType_ == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
}

template <typename LockConfigT>
template <typename AllocConfigT, OSPagesAllocPolicy OS_ALLOC_POLICY>
Region *RegionAllocatorBase<LockConfigT>::CreateAndSetUpNewRegion(size_t regionSize, RegionFlag regionType,
                                                                  RegionFlag properties)
{
    Region *region = AllocRegion<OS_ALLOC_POLICY>(regionSize, regionType, properties);
    if (LIKELY(region != nullptr)) {
        if (regionType == RegionFlag::IS_EDEN) {
            AllocConfigT::OnInitYoungRegion({region->Begin(), region->End()});
        }
        // Do memory barrier here to make sure all threads see references to bitmaps.
        // The situation:
        // A mutator thread allocates a new object. During object allocation the mutator
        // allocates a new region, sets up the region header, allocates object in the region and publishes
        // the reference to the object.
        // GC thread does concurrent marking. It sees the reference to the new object and gets the region
        // by the object address.
        // Since GC thread doesn't locks region_lock_ we need to do memory barrier here to make
        // sure GC thread sees all bitmaps from the region header.
        arch::FullMemoryBarrier();
        // Getting region by object is a bit operation and TSAN doesn't
        // sees the relation between region creation and region access.
        // This annotation suggests TSAN that this code always executes before
        // the region will be accessed.
        // See the corresponding annotation in ObjectToRegion
        TSAN_ANNOTATE_HAPPENS_BEFORE(region);
    }
    return region;
}

template <typename LockConfigT>
PandaVector<Region *> RegionAllocatorBase<LockConfigT>::GetAllRegions()
{
    PandaVector<Region *> vector;
    os::memory::LockHolder lock(this->regionLock_);
    GetSpace()->IterateRegions([&](Region *region) { vector.push_back(region); });
    return vector;
}

template <typename LockConfigT>
double RegionAllocatorBase<LockConfigT>::CalculateDeadObjectsRatio()
{
    os::memory::LockHolder lock(this->regionLock_);
    size_t totalSize = 0;
    size_t deadObjectSize = 0;
    size_t count = 0;
    GetSpace()->IterateRegions([&count, &totalSize, &deadObjectSize](Region *region) {
        count++;
        totalSize += region->Size();
        deadObjectSize += region->GetGarbageBytes();
    });
    if (totalSize == 0) {
        return 0;
    }
    return static_cast<double>(deadObjectSize) / totalSize;
}

template <typename AllocConfigT, typename LockConfigT>
RegionAllocator<AllocConfigT, LockConfigT>::RegionAllocator(MemStatsType *memStats, GenerationalSpaces *spaces,
                                                            SpaceType spaceType, size_t initSpaceSize, bool extend,
                                                            size_t emptyTenuredRegionsMaxCount)
    : RegionAllocatorBase<LockConfigT>(memStats, spaces, spaceType, AllocatorType::REGION_ALLOCATOR, initSpaceSize,
                                       extend, REGION_SIZE, emptyTenuredRegionsMaxCount),
      fullRegion_(nullptr, 0, 0),
      edenCurrentRegion_(&fullRegion_)
{
}

template <typename AllocConfigT, typename LockConfigT>
RegionAllocator<AllocConfigT, LockConfigT>::RegionAllocator(MemStatsType *memStats, GenerationalSpaces *spaces,
                                                            SpaceType spaceType, RegionPool *sharedRegionPool,
                                                            size_t emptyTenuredRegionsMaxCount)
    : RegionAllocatorBase<LockConfigT>(memStats, spaces, spaceType, AllocatorType::REGION_ALLOCATOR, sharedRegionPool,
                                       emptyTenuredRegionsMaxCount),
      fullRegion_(nullptr, 0, 0),
      edenCurrentRegion_(&fullRegion_)
{
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGION_TYPE>
RegionMem RegionAllocator<AllocConfigT, LockConfigT>::AllocRegular(size_t alignSize)
{
    static constexpr bool IS_ATOMIC = std::is_same_v<LockConfigT, RegionAllocatorLockConfig::CommonLock>;
    bool isRegionZeroed = false;
    static_assert((REGION_TYPE == RegionFlag::IS_EDEN) || (REGION_TYPE == RegionFlag::IS_OLD) ||
                  (REGION_TYPE == RegionFlag::IS_PINNED));
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
        Region *region = GetCurrentRegion<IS_ATOMIC, REGION_TYPE>();
        void *mem = region->template Alloc<IS_ATOMIC>(alignSize);
        if (mem != nullptr) {
            return RegionMem {mem, region->HasFlag(RegionFlag::IS_ZEROED)};
        }

        os::memory::LockHolder lock(this->regionLock_);
        region = GetCurrentRegion<IS_ATOMIC, REGION_TYPE>();
        mem = region->template Alloc<IS_ATOMIC>(alignSize);
        if (mem != nullptr) {
            return RegionMem {mem, region->HasFlag(RegionFlag::IS_ZEROED)};
        }

        region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, REGION_TYPE);
        if (LIKELY(region != nullptr)) {
            // Here we need memory barrier to make the allocation visible
            // in all threads before SetCurrentRegion
            mem = region->template Alloc<IS_ATOMIC>(alignSize);
            isRegionZeroed = region->HasFlag(RegionFlag::IS_ZEROED);
            SetCurrentRegion<IS_ATOMIC, REGION_TYPE>(region);
        }

        return RegionMem {mem, isRegionZeroed};
    }
    void *mem = nullptr;
    Region *regionTo = PopFromRegionQueue<IS_ATOMIC, REGION_TYPE>();
    if (regionTo != nullptr) {
        // Here we need memory barrier to make the allocation visible
        // in all threads before SetCurrentRegion
        mem = regionTo->template Alloc<IS_ATOMIC>(alignSize);
        if (mem != nullptr) {
            if constexpr (REGION_TYPE == RegionFlag::IS_PINNED) {
                regionTo->PinObject();
            }
            PushToRegionQueue<IS_ATOMIC, REGION_TYPE>(regionTo);
            return RegionMem {mem, regionTo->HasFlag(RegionFlag::IS_ZEROED)};
        }
    }

    os::memory::LockHolder lock(this->regionLock_);
    RegionFlag properties = (REGION_TYPE == RegionFlag::IS_PINNED) ? RegionFlag::IS_PINNED : RegionFlag::IS_UNUSED;
    regionTo = this->template CreateAndSetUpNewRegion<AllocConfigT, OSPagesAllocPolicy::ZEROED_MEMORY>(
        REGION_SIZE, RegionFlag::IS_OLD, properties);
    if (LIKELY(regionTo != nullptr)) {
        // Here we need memory barrier to make the allocation visible
        // in all threads before SetCurrentRegion
        mem = regionTo->template Alloc<IS_ATOMIC>(alignSize);
        isRegionZeroed = regionTo->HasFlag(RegionFlag::IS_ZEROED);
        if constexpr (REGION_TYPE == RegionFlag::IS_PINNED) {
            regionTo->PinObject();
        }
        PushToRegionQueue<IS_ATOMIC, REGION_TYPE>(regionTo);
    }
    return RegionMem {mem, isRegionZeroed};
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGION_TYPE, bool UPDATE_MEMSTATS>
void *RegionAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align, bool pinned)
{
    RegionMem regionMem = AllocExt<REGION_TYPE, UPDATE_MEMSTATS>(size, align, pinned);
    return regionMem.mem;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGION_TYPE, bool UPDATE_MEMSTATS>
RegionMem RegionAllocator<AllocConfigT, LockConfigT>::AllocExt(size_t size, Alignment align, bool pinned)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t alignSize = AlignUp(size, GetAlignmentInBytes(align));
    RegionMem regionMem {nullptr, false};
    // for movable & regular size object, allocate it from a region
    // for nonmovable or large size object, allocate a seprate large region for it
    if (this->GetSpaceType() != SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT &&
        LIKELY(alignSize <= GetMaxRegularObjectSize())) {
        regionMem = pinned ? AllocRegular<IS_PINNED>(alignSize) : AllocRegular<REGION_TYPE>(alignSize);
    } else {
        os::memory::LockHolder lock(this->regionLock_);
        Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(
            Region::RegionSize(alignSize, REGION_SIZE), REGION_TYPE, IS_LARGE_OBJECT);
        if (LIKELY(region != nullptr)) {
            regionMem = RegionMem {region->Alloc<false>(alignSize), region->HasFlag(RegionFlag::IS_ZEROED)};
        }
    }
    if (regionMem.mem != nullptr) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (UPDATE_MEMSTATS) {
            AllocConfigT::OnAlloc(alignSize, this->spaceType_, this->memStats_);
            if (!regionMem.isZeroed) {
                AllocConfigT::MemoryInit(regionMem.mem);
            }
        }
        // Do it after memory init because we can reach this memory after setting live bitmap
        if ((REGION_TYPE == RegionFlag::IS_OLD) || pinned) {
            auto liveBitmap = this->GetRegion(reinterpret_cast<ObjectHeader *>(regionMem.mem))->GetLiveBitmap();
            ASSERT(liveBitmap != nullptr);
            liveBitmap->AtomicTestAndSet(regionMem.mem);
        }
    }
    return regionMem;
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::PinObject(ObjectHeader *object)
{
    auto *region = ObjectToRegion(object);
    ASSERT(region != nullptr);
    region->PinObject();
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::UnpinObject(ObjectHeader *object)
{
    auto *region = ObjectToRegion(object);
    ASSERT(region != nullptr);
    region->UnpinObject();
    if (!region->HasPinnedObjects()) {
        static constexpr bool IS_ATOMIC = std::is_same_v<LockConfigT, RegionAllocatorLockConfig::CommonLock>;
        PandaVector<Region *> *regionQueue = GetRegionQueuePointer<RegionFlag::IS_PINNED>();
        os::memory::LockHolder<LockConfigT, IS_ATOMIC> lock(*GetQueueLock<RegionFlag::IS_PINNED>());
        auto itRegion = std::find(regionQueue->begin(), regionQueue->end(), region);
        if (itRegion != regionQueue->end()) {
            regionQueue->erase(itRegion);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateTLAB(size_t size)
{
    ASSERT(size <= GetMaxRegularObjectSize());
    ASSERT(AlignUp(size, GetAlignmentInBytes(DEFAULT_ALIGNMENT)) == size);
    TLAB *tlab = nullptr;

    {
        os::memory::LockHolder lock(this->regionLock_);
        Region *region = nullptr;
        // first search in partial tlab map
        auto largestTlab = retainedTlabs_.begin();
        if (largestTlab != retainedTlabs_.end() && largestTlab->first >= size) {
            LOG(DEBUG, ALLOC) << "Use retained tlabs region " << region;
            region = largestTlab->second;
            retainedTlabs_.erase(largestTlab);
            ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
        }

        // allocate a free region if none partial tlab has enough space
        if (region == nullptr) {
            region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_EDEN);
            if (LIKELY(region != nullptr)) {
                region->CreateTLABSupport();
            }
        }
        if (region != nullptr) {
            tlab = CreateTLABInRegion(region, size);
            auto remainingSize = region->GetRemainingSizeForTLABs();
            if (remainingSize >= size) {
                LOG(DEBUG, ALLOC) << "Add a region " << region << " with remained size " << remainingSize
                                  << " to retained_tlabs";
                retainedTlabs_.insert(std::make_pair(remainingSize, region));
            }
        }
    }

    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateRegionSizeTLAB()
{
    TLAB *tlab = nullptr;

    os::memory::LockHolder lock(this->regionLock_);
    Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_EDEN);
    if (LIKELY(region != nullptr)) {
        region->CreateTLABSupport();
        size_t size = region->GetRemainingSizeForTLABs();
        tlab = CreateTLABInRegion(region, size);
    }

    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
TLAB *RegionAllocator<AllocConfigT, LockConfigT>::CreateTLABInRegion(Region *region, size_t size)
{
    // We don't reuse the same region for different TLABs.
    // Therefore, update the size
    TLAB *tlab = region->CreateTLAB(size);
    ASSERT(tlab != nullptr);
    LOG(DEBUG, ALLOC) << "Found a region " << region << " and create tlab " << tlab << " with memory starts at "
                      << tlab->GetStartAddr() << " and with size " << tlab->GetSize();
    return tlab;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool INCLUDE_CURRENT_REGION>
void RegionAllocator<AllocConfigT, LockConfigT>::GetTopGarbageRegions(double garbageThreshold,
                                                                      GarbageRegions &garbageRegions,
                                                                      PandaVector<Region *> &emptyRegions)
{
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (region->HasFlag(IS_EDEN) || region->HasFlag(RegionFlag::IS_RESERVED)) {
            return;
        }
        if constexpr (!INCLUDE_CURRENT_REGION) {
            if (IsInCurrentRegion<true, RegionFlag::IS_OLD>(region)) {
                return;
            }
        }
        auto garbageBytes = region->GetGarbageBytes();
        if (region->GetLiveBytes() == 0U) {
            emptyRegions.push_back(region);
        } else if (static_cast<double>(garbageBytes) / region->GetAllocatedBytes() >= garbageThreshold) {
            garbageRegions.emplace_back(std::make_pair(garbageBytes, region));
        }
    });
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE>
PandaVector<Region *> RegionAllocator<AllocConfigT, LockConfigT>::GetAllSpecificRegions()
{
    PandaVector<Region *> vector;
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (region->HasFlag(REGIONS_TYPE)) {
            vector.push_back(region);
        }
    });
    return vector;
}

template <typename AllocConfigT, typename LockConfigT>
double RegionAllocator<AllocConfigT, LockConfigT>::CalculateInternalOldFragmentation()
{
    constexpr RegionFlag REGIONS_TYPE = RegionFlag::IS_OLD;
    size_t totalFreeSize = 0;
    size_t totalAllocatedSize = 0;
    this->GetSpace()->IterateRegions([this, &totalFreeSize, &totalAllocatedSize](Region *region) {
        if (!region->HasFlag(REGIONS_TYPE)) {
            return;
        }
        if (IsInCurrentRegion<false, REGIONS_TYPE>(region)) {
            return;
        }
        size_t allocatedBytes = region->GetAllocatedBytes();
        size_t regionSize = region->Size();
        ASSERT(regionSize >= allocatedBytes);
        totalFreeSize += regionSize - allocatedBytes;
        totalAllocatedSize += allocatedBytes;
    });
    if (totalAllocatedSize == 0) {
        return 0;
    }
    return static_cast<double>(totalFreeSize) / totalAllocatedSize;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactAllSpecificRegions(const GCObjectVisitor &deathChecker,
                                                                           const ObjectVisitorEx &moveHandler)
{
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {  // NOLINT(bugprone-suspicious-semicolon)
        // NOTE(aemelenko): Implement it if need to call this method with the same regions type.
        // There is an issue with IterateRegions during creating a new one.
        ASSERT(REGIONS_TYPE_FROM != REGIONS_TYPE_TO);
        ResetCurrentRegion<false, REGIONS_TYPE_TO>();
    }
    this->GetSpace()->IterateRegions([this, &deathChecker, &moveHandler](Region *region) {
        if (!region->HasFlag(REGIONS_TYPE_FROM)) {
            return;
        }
        CompactSpecificRegion<REGIONS_TYPE_FROM, REGIONS_TYPE_TO, USE_MARKED_BITMAP>(region, deathChecker, moveHandler);
    });
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactSeveralSpecificRegions(const PandaVector<Region *> &regions,
                                                                               const GCObjectVisitor &deathChecker,
                                                                               const ObjectVisitorEx &moveHandler)
{
    for (auto i : regions) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {
            [[maybe_unused]] bool foundedRegion = IsInCurrentRegion<false, REGIONS_TYPE_TO>(i);
            ASSERT(!foundedRegion);
        }
        CompactSpecificRegion<REGIONS_TYPE_FROM, REGIONS_TYPE_TO, USE_MARKED_BITMAP>(i, deathChecker, moveHandler);
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP>
void RegionAllocator<AllocConfigT, LockConfigT>::CompactSpecificRegion(Region *region,
                                                                       const GCObjectVisitor &deathChecker,
                                                                       const ObjectVisitorEx &moveHandler)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (REGIONS_TYPE_FROM == REGIONS_TYPE_TO) {
        // It is bad if we compact one region into itself.
        [[maybe_unused]] bool isCurrentRegion = IsInCurrentRegion<true, REGIONS_TYPE_TO>(region);
        ASSERT(!isCurrentRegion);
    }
    auto createNewRegion = [&]() {
        os::memory::LockHolder lock(this->regionLock_);
        Region *regionTo = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, REGIONS_TYPE_TO);
        ASSERT(regionTo != nullptr);
        return regionTo;
    };

    Region *regionTo = PopFromRegionQueue<true, REGIONS_TYPE_TO>();
    if (regionTo == nullptr) {
        regionTo = createNewRegion();
    }
    size_t liveBytes = 0;
    // Don't use atomic in this method because we work with not shared region
    auto visitor = [&](ObjectHeader *object) {
        // If we use mark-bitmap then we iterate over alive object, so no need death-checker
        if constexpr (!USE_MARKED_BITMAP) {
            if (deathChecker(object) != ObjectStatus::ALIVE_OBJECT) {
                return;
            }
        }
        size_t objectSize = GetObjectSize(object);
        size_t alignedSize = AlignUp(objectSize, DEFAULT_ALIGNMENT_IN_BYTES);
        void *dst = regionTo->template Alloc<false>(alignedSize);
        if (dst == nullptr) {
            regionTo->SetLiveBytes(regionTo->GetLiveBytes() + liveBytes);
            liveBytes = 0;
            regionTo = createNewRegion();
            dst = regionTo->template Alloc<false>(alignedSize);
        }
        // Don't initialize memory for an object here because we will use memcpy anyway
        ASSERT(dst != nullptr);
        MemcpyUnsafe(dst, object, objectSize);
        // need to mark as alive moved object
        ASSERT(regionTo->GetLiveBitmap() != nullptr);
        regionTo->IncreaseAllocatedObjects();
        regionTo->GetLiveBitmap()->Set(dst);
        liveBytes += alignedSize;
        moveHandler(object, static_cast<ObjectHeader *>(dst));
    };

    ASSERT(region->HasFlag(REGIONS_TYPE_FROM));

    const std::function<void(ObjectHeader *)> visitorFunctor(visitor);
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (USE_MARKED_BITMAP) {
        region->GetMarkBitmap()->IterateOverMarkedChunks(
            [&visitorFunctor](void *objectAddr) { visitorFunctor(static_cast<ObjectHeader *>(objectAddr)); });
    } else {  // NOLINT(readability-misleading-indentation)
        region->IterateOverObjects(visitorFunctor);
    }
    regionTo->SetLiveBytes(regionTo->GetLiveBytes() + liveBytes);

    PushToRegionQueue<true, REGIONS_TYPE_TO>(regionTo);
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::ReserveRegionIfNeeded()
{
    if (reservedRegion_ != nullptr) {
        return;
    }
    reservedRegion_ = this->GetSpace()->NewRegion(REGION_SIZE, RegionFlag::IS_OLD, RegionFlag::IS_RESERVED);
    ASSERT(reservedRegion_ != nullptr);
    reservedRegion_->RmvFlag(RegionFlag::IS_OLD);
}

template <typename AllocConfigT, typename LockConfigT>
void RegionAllocator<AllocConfigT, LockConfigT>::ReleaseReservedRegion()
{
    ASSERT(reservedRegion_ != nullptr);
    this->GetSpace()->template FreeRegion<RegionSpace::ReleaseRegionsPolicy::NoRelease, OSPagesPolicy::NO_RETURN>(
        reservedRegion_);
    reservedRegion_ = nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool USE_MARKED_BITMAP, bool FULL_GC>
size_t RegionAllocator<AllocConfigT, LockConfigT>::PromoteYoungRegion(Region *region,
                                                                      const GCObjectVisitor &deathChecker,
                                                                      const ObjectVisitor &aliveObjectsHandler)
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    size_t aliveMoveCount = 0;
    // We should create live bitmap here and copy alive object in marked bitmap to it
    region->CreateLiveBitmap();
    region->CloneMarkBitmapToLiveBitmap();
    [[maybe_unused]] auto visitor = [&aliveObjectsHandler, &region](ObjectHeader *object) {
        aliveObjectsHandler(object);
        region->IncreaseAllocatedObjects();
    };
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (USE_MARKED_BITMAP) {
        if constexpr (FULL_GC) {
            region->GetMarkBitmap()->IterateOverMarkedChunks(
                [&visitor](void *objectAddr) { visitor(static_cast<ObjectHeader *>(objectAddr)); });
        } else {
            aliveMoveCount = region->UpdateAllocatedObjects();
        }
    } else {  // NOLINT(readability-misleading-indentation)
        auto liveCheckVisitor = [&visitor, &deathChecker](ObjectHeader *object) {
            if (deathChecker(object) == ObjectStatus::ALIVE_OBJECT) {
                visitor(object);
            }
        };
        region->IterateOverObjects(liveCheckVisitor);
    }
    // We set not actual value here but we will update it later
    region->SetLiveBytes(region->GetAllocatedBytes());
    this->GetSpace()->PromoteYoungRegion(region);
    return aliveMoveCount;
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE>
void RegionAllocator<AllocConfigT, LockConfigT>::ResetAllSpecificRegions()
{
    ResetCurrentRegion<false, REGIONS_TYPE>();
    this->GetSpace()->IterateRegions([&](Region *region) {
        if (!region->HasFlag(REGIONS_TYPE)) {
            return;
        }
        this->GetSpace()->template FreeRegion<RegionSpace::ReleaseRegionsPolicy::NoRelease>(region);
    });
    if constexpr (REGIONS_TYPE == RegionFlag::IS_EDEN) {
        retainedTlabs_.clear();
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
          OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
void RegionAllocator<AllocConfigT, LockConfigT>::ResetSeveralSpecificRegions(const Container &regions)
{
    os::memory::LockHolder<LockConfigT, NEED_LOCK> lock(this->regionLock_);
    ASSERT(REGIONS_TYPE != RegionFlag::IS_EDEN);
    ASSERT((REGIONS_TYPE != RegionFlag::IS_EDEN) || (retainedTlabs_.empty()));
    for (Region *region : regions) {
        ASSERT(!(IsInCurrentRegion<false, REGIONS_TYPE>(region)));
        ASSERT(region->HasFlag(REGIONS_TYPE));
        this->GetSpace()->template FreeRegion<REGIONS_RELEASE_POLICY, OS_PAGES_POLICY>(region);
    }
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::RegionNonmovableAllocator(
    MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType, size_t initSpaceSize, bool extend)
    : RegionAllocatorBase<LockConfigT>(memStats, spaces, spaceType, ObjectAllocator::GetAllocatorType(), initSpaceSize,
                                       extend, REGION_SIZE, 0),
      objectAllocator_(memStats, spaceType)
{
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::RegionNonmovableAllocator(
    MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType, RegionPool *sharedRegionPool)
    : RegionAllocatorBase<LockConfigT>(memStats, spaces, spaceType, ObjectAllocator::GetAllocatorType(),
                                       sharedRegionPool, 0),
      objectAllocator_(memStats, spaceType)
{
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void *RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Alloc(size_t size, Alignment align)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t alignSize = AlignUp(size, GetAlignmentInBytes(align));
    ASSERT(alignSize <= ObjectAllocator::GetMaxSize());

    void *mem = objectAllocator_.Alloc(alignSize);
    if (UNLIKELY(mem == nullptr)) {
        mem = NewRegionAndRetryAlloc(size, align);
        if (UNLIKELY(mem == nullptr)) {
            return nullptr;
        }
    }
    auto liveBitmap = this->GetRegion(reinterpret_cast<ObjectHeader *>(mem))->GetLiveBitmap();
    ASSERT(liveBitmap != nullptr);
    liveBitmap->AtomicTestAndSet(mem);
    return mem;
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Free(void *mem)
{
    this->GetRegion(reinterpret_cast<ObjectHeader *>(mem))->GetLiveBitmap()->AtomicTestAndClear(mem);

    objectAllocator_.Free(mem);
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::Collect(const GCObjectVisitor &deathChecker)
{
    os::memory::LockHolder lock(this->regionLock_);
    objectAllocator_.Collect(deathChecker);
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::VisitAndRemoveFreeRegions(
    const RegionsVisitor &regionVisitor)
{
    os::memory::LockHolder lock(this->regionLock_);
    // Add free region into vector to not do extra work with region_visitor
    // inside object_allocator_.
    PandaVector<Region *> freeRegions;

    objectAllocator_.VisitAndRemoveFreePools([&freeRegions](void *mem, [[maybe_unused]] size_t size) {
        auto *region = AddrToRegion(mem);
        ASSERT(ToUintPtr(mem) + size == region->End());
        // We don't remove this region here, because don't want to do some extra work with visitor here.
        freeRegions.push_back(region);
    });

    if (!freeRegions.empty()) {
        regionVisitor(freeRegions);

        for (auto i : freeRegions) {
            this->GetSpace()->FreeRegion(i);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
void *RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::NewRegionAndRetryAlloc(size_t objectSize,
                                                                                                    Alignment align)
{
    os::memory::LockHolder lock(this->regionLock_);
    size_t poolHeadSize = AlignUp(Region::HeadSize(), ObjectAllocator::PoolAlign());
    ASSERT(AlignUp(poolHeadSize + objectSize, REGION_SIZE) == REGION_SIZE);
    while (true) {
        Region *region = this->template CreateAndSetUpNewRegion<AllocConfigT>(REGION_SIZE, RegionFlag::IS_NONMOVABLE);
        if (UNLIKELY(region == nullptr)) {
            return nullptr;
        }
        ASSERT(region->GetLiveBitmap() != nullptr);
        uintptr_t alignedPool = ToUintPtr(region) + poolHeadSize;
        bool addedMemoryPool = objectAllocator_.AddMemoryPool(ToVoidPtr(alignedPool), REGION_SIZE - poolHeadSize);
        ASSERT(addedMemoryPool);
        if (UNLIKELY(!addedMemoryPool)) {
            LOG(FATAL, ALLOC) << "ObjectAllocator: couldn't add memory pool to allocator";
        }
        void *mem = objectAllocator_.Alloc(objectSize, align);
        if (LIKELY(mem != nullptr)) {
            return mem;
        }
    }
    return nullptr;
}

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
double RegionNonmovableAllocator<AllocConfigT, LockConfigT, ObjectAllocator>::CalculateDeadObjectsRatio()
{
#ifdef PANDA_MEASURE_FRAGMENTATION
    os::memory::LockHolder lock(this->regionLock_);
    size_t totalSize = 0;
    size_t liveObjectSize = 0;
    size_t count = 0;
    this->GetSpace()->IterateRegions([&count, &totalSize, &liveObjectSize](Region *region) {
        count++;
        totalSize += region->Size();
        liveObjectSize += region->GetLiveBytes();
    });
    if (totalSize == 0) {
        return 0;
    }
    auto allocatedBytes = objectAllocator_.GetAllocatedBytes();
    ASSERT(allocatedBytes >= liveObjectSize);
    auto deadObjectSize = allocatedBytes - liveObjectSize;
    return static_cast<double>(deadObjectSize) / totalSize;
#else
    LOG(FATAL, ALLOC) << "Not implemented. Requires build with define PANDA_MEASURE_FRAGMENTATION";
    UNREACHABLE();
#endif  // #ifdef PANDA_MEASURE_FRAGMENTATION
}

template <typename AllocConfigT, typename LockConfigT>
RegionHumongousAllocator<AllocConfigT, LockConfigT>::RegionHumongousAllocator(MemStatsType *memStats,
                                                                              GenerationalSpaces *spaces,
                                                                              SpaceType spaceType)
    : RegionAllocatorBase<LockConfigT>(memStats, spaces, spaceType, AllocatorType::REGION_ALLOCATOR, 0, true,
                                       REGION_SIZE, 0)
{
}

template <typename AllocConfigT, typename LockConfigT>
template <bool UPDATE_MEMSTATS>
void *RegionHumongousAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align)
{
    ASSERT(GetAlignmentInBytes(align) % GetAlignmentInBytes(DEFAULT_ALIGNMENT) == 0);
    size_t alignSize = AlignUp(size, GetAlignmentInBytes(align));
    Region *region = nullptr;
    void *mem = nullptr;
    // allocate a seprate large region for object
    {
        os::memory::LockHolder lock(this->regionLock_);
        region = this->template CreateAndSetUpNewRegion<AllocConfigT, OSPagesAllocPolicy::ZEROED_MEMORY>(
            Region::RegionSize(alignSize, REGION_SIZE), IS_OLD, IS_LARGE_OBJECT);
        if (LIKELY(region != nullptr)) {
            mem = region->Alloc<false>(alignSize);
            ASSERT(mem != nullptr);
            ASSERT(region->GetLiveBitmap() != nullptr);
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (UPDATE_MEMSTATS) {
                AllocConfigT::OnAlloc(region->Size(), this->spaceType_, this->memStats_);
                // We don't set up memory here because the requested memory should
                // be zeroed
            }
            // Do it after memory init because we can reach this memory after setting live bitmap
            region->GetLiveBitmap()->AtomicTestAndSet(mem);
        }
    }
    return mem;
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::CollectAndRemoveFreeRegions(
    const RegionsVisitor &regionVisitor, const GCObjectVisitor &deathChecker)
{
    // Add free region into vector to not do extra work with region_visitor during region iteration
    PandaVector<Region *> freeRegions;

    {
        os::memory::LockHolder lock(this->regionLock_);
        this->GetSpace()->IterateRegions([this, &deathChecker, &freeRegions](Region *region) {
            this->Collect(region, deathChecker);
            if (region->HasFlag(IS_FREE)) {
                freeRegions.push_back(region);
            }
        });
    }

    if (!freeRegions.empty()) {
        regionVisitor(freeRegions);

        for (auto i : freeRegions) {
            os::memory::LockHolder lock(this->regionLock_);
            ResetRegion(i);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::Collect(Region *region, const GCObjectVisitor &deathChecker)
{
    ASSERT(region->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ObjectHeader *objectToProceed = nullptr;
    objectToProceed = region->GetLargeObject();
    if (deathChecker(objectToProceed) == ObjectStatus::DEAD_OBJECT) {
        region->AddFlag(RegionFlag::IS_FREE);
    }
}

template <typename AllocConfigT, typename LockConfigT>
void RegionHumongousAllocator<AllocConfigT, LockConfigT>::ResetRegion(Region *region)
{
    ASSERT(region->HasFlag(RegionFlag::IS_FREE));
    this->GetSpace()->FreeRegion(region);
}

template <typename AllocConfigT, typename LockConfigT>
double RegionHumongousAllocator<AllocConfigT, LockConfigT>::CalculateInternalFragmentation()
{
    size_t totalFreeSize = 0;
    size_t totalAllocatedSize = 0;
    this->GetSpace()->IterateRegions([&totalFreeSize, &totalAllocatedSize](Region *region) {
        auto allocatedBytes = region->GetAllocatedBytes();
        auto regionSize = region->Size();
        ASSERT(regionSize >= allocatedBytes);
        auto freeSize = regionSize - allocatedBytes;
        totalFreeSize += freeSize;
        totalAllocatedSize += allocatedBytes;
    });
    if (totalAllocatedSize == 0) {
        return 0;
    }
    return static_cast<double>(totalFreeSize) / totalAllocatedSize;
}

template <typename AllocConfigT, typename LockConfigT>
using RegionRunslotsAllocator = RegionNonmovableAllocator<AllocConfigT, LockConfigT, RunSlotsAllocator<AllocConfigT>>;

template <typename AllocConfigT, typename LockConfigT>
using RegionFreeListAllocator = RegionNonmovableAllocator<AllocConfigT, LockConfigT, FreeListAllocator<AllocConfigT>>;

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_REGION_ALLOCATOR_INL_H
