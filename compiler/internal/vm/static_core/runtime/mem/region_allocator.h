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
#ifndef PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H
#define PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H

#include <atomic>
#include <cstdint>

#include "runtime/mem/region_space.h"

namespace ark {
class ManagedThread;
struct GCTask;
}  // namespace ark

namespace ark::mem {

class RegionAllocatorLockConfig {
public:
    using CommonLock = os::memory::Mutex;
    using DummyLock = os::memory::DummyLock;
};

using RegionsVisitor = std::function<void(PandaVector<Region *> &vector)>;

struct RegionMem {
    void *mem;
    bool isZeroed;
};

/// Return the region which corresponds to the start of the object.
static inline Region *ObjectToRegion(const ObjectHeader *object)
{
    auto *region = reinterpret_cast<Region *>(((ToUintPtr(object)) & ~DEFAULT_REGION_MASK));
    ASSERT(ToUintPtr(PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(object)) == ToUintPtr(region));
    // Getting region by object is a bit operation and TSAN doesn't
    // sees the relation between region creation and region access.
    // This annotation suggests TSAN that this code always executes after
    // the region gets created.
    // See the corresponding annotation in RegionAllocatorBase::CreateAndSetUpNewRegion
    TSAN_ANNOTATE_HAPPENS_AFTER(region);
    return region;
}

static inline bool IsSameRegion(const void *o1, const void *o2, size_t regionSizeBits)
{
    return ((ToUintPtr(o1) ^ ToUintPtr(o2)) >> regionSizeBits) == 0;
}

/// Return the region which corresponds to the address.
static inline Region *AddrToRegion(const void *addr)
{
    auto regionAddr = PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(addr);
    return static_cast<Region *>(regionAddr);
}

template <typename LockConfigT>
class RegionAllocatorBase {
public:
    NO_MOVE_SEMANTIC(RegionAllocatorBase);
    NO_COPY_SEMANTIC(RegionAllocatorBase);

    explicit RegionAllocatorBase(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType,
                                 AllocatorType allocatorType, size_t initSpaceSize, bool extend, size_t regionSize,
                                 size_t emptyTenuredRegionsMaxCount);
    explicit RegionAllocatorBase(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType,
                                 AllocatorType allocatorType, RegionPool *sharedRegionPool,
                                 size_t emptyTenuredRegionsMaxCount);

    virtual ~RegionAllocatorBase()
    {
        ClearRegionsPool();
    }

    Region *GetRegion(const ObjectHeader *object) const
    {
        return regionSpace_.GetRegion(object);
    }

    RegionSpace *GetSpace()
    {
        return &regionSpace_;
    }

    const RegionSpace *GetSpace() const
    {
        return &regionSpace_;
    }

    PandaVector<Region *> GetAllRegions();

    template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
    void ReleaseEmptyRegions()
    {
        this->GetSpace()->template ReleaseEmptyRegions<REGION_TYPE, OS_PAGES_POLICY>();
    }

    virtual double CalculateDeadObjectsRatio();

protected:
    void ClearRegionsPool()
    {
        regionSpace_.FreeAllRegions();

        if (initBlock_.GetMem() != nullptr) {
            spaces_->FreeSharedPool(initBlock_.GetMem(), initBlock_.GetSize());
            initBlock_ = NULLPOOL;
        }
    }

    template <OSPagesAllocPolicy OS_ALLOC_POLICY>
    Region *AllocRegion(size_t regionSize, RegionFlag edenOrOldOrNonmovable, RegionFlag properties)
    {
        return regionSpace_.NewRegion(regionSize, edenOrOldOrNonmovable, properties, OS_ALLOC_POLICY);
    }

    SpaceType GetSpaceType() const
    {
        return spaceType_;
    }

    template <typename AllocConfigT, OSPagesAllocPolicy OS_ALLOC_POLICY = OSPagesAllocPolicy::NO_POLICY>
    Region *CreateAndSetUpNewRegion(size_t regionSize, RegionFlag regionType, RegionFlag properties = IS_UNUSED)
        REQUIRES(regionLock_);

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    LockConfigT regionLock_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MemStatsType *memStats_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    SpaceType spaceType_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    GenerationalSpaces *spaces_;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RegionPool regionPool_;  // self created pool, only used by this allocator
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RegionSpace regionSpace_;  // the target region space used by this allocator
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Pool initBlock_;  // the initial memory block for region allocation
};

/// @brief A region-based bump-pointer allocator.
template <typename AllocConfigT, typename LockConfigT = RegionAllocatorLockConfig::CommonLock>
class RegionAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    using GarbageRegions = PandaVector<std::pair<uint32_t, Region *>>;

    static constexpr bool USE_PARTIAL_TLAB = true;
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionAllocator);
    NO_COPY_SEMANTIC(RegionAllocator);

    /**
     * @brief Create new region allocator
     * @param mem_stats - memory statistics
     * @param space_type - space type
     * @param init_space_size - initial continuous space size, 0 means no need for initial space
     * @param extend - true means that will allocate more regions from mmap pool if initial space is not enough
     */
    explicit RegionAllocator(MemStatsType *memStats, GenerationalSpaces *spaces,
                             SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT, size_t initSpaceSize = 0,
                             bool extend = true, size_t emptyTenuredRegionsMaxCount = 0);

    /**
     * @brief Create new region allocator with shared region pool specified
     * @param mem_stats - memory statistics
     * @param space_type - space type
     * @param shared_region_pool - a shared region pool that can be reused by multi-spaces
     */
    explicit RegionAllocator(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType,
                             RegionPool *sharedRegionPool, size_t emptyTenuredRegionsMaxCount = 0);

    ~RegionAllocator() override = default;

    template <RegionFlag REGION_TYPE = RegionFlag::IS_EDEN, bool UPDATE_MEMSTATS = true>
    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT, bool pinned = false);
    template <RegionFlag REGION_TYPE = RegionFlag::IS_EDEN, bool UPDATE_MEMSTATS = true>
    struct RegionMem AllocExt(size_t size, Alignment align = DEFAULT_ALIGNMENT, bool pinned = false);

    template <typename T>
    T *AllocArray(size_t arrLength)
    {
        return static_cast<T *>(Alloc(sizeof(T) * arrLength));
    }

    void Free([[maybe_unused]] void *mem) {}

    void PinObject(ObjectHeader *object);

    void UnpinObject(ObjectHeader *object);

    /**
     * @brief Create a TLAB of the specified size
     * @param size - required size of tlab
     * @return newly allocated TLAB, TLAB is set to Empty is allocation failed.
     */
    TLAB *CreateTLAB(size_t size);

    /**
     * @brief Create a TLAB in a new region. TLAB will occupy the whole region.
     * @return newly allocated TLAB, TLAB is set to Empty is allocation failed.
     */
    TLAB *CreateRegionSizeTLAB();

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @param visitor - function pointer or functor
     */
    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &visitor)
    {
        this->GetSpace()->IterateRegions([&](Region *region) { region->IterateOverObjects(visitor); });
    }

    template <typename ObjectVisitor>
    void IterateOverObjectsInRange(const ObjectVisitor &visitor, void *begin, void *end)
    {
        this->GetSpace()->IterateRegions([&](Region *region) {
            if (!region->Intersect(ToUintPtr(begin), ToUintPtr(end))) {
                return;
            }
            region->IterateOverObjects([&visitor, begin, end](ObjectHeader *obj) {
                if (ToUintPtr(begin) <= ToUintPtr(obj) && ToUintPtr(obj) < ToUintPtr(end)) {
                    visitor(obj);
                }
            });
        });
    }

    template <bool INCLUDE_CURRENT_REGION>
    void GetTopGarbageRegions(double garbageThreshold, GarbageRegions &garbageRegions,
                              PandaVector<Region *> &emptyRegions);

    /**
     * Return a vector of all regions with the specific type.
     * @tparam regions_type - type of regions needed to proceed.
     * @return vector of all regions with the /param regions_type type
     */
    template <RegionFlag REGIONS_TYPE>
    PandaVector<Region *> GetAllSpecificRegions();

    double CalculateInternalOldFragmentation();

    /**
     * Iterate over all regions with type /param regions_type_from
     * and move all alive objects to the regions with type /param regions_type_to.
     * NOTE: /param regions_type_from and /param regions_type_to can't be equal.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactAllSpecificRegions(const GCObjectVisitor &deathChecker, const ObjectVisitorEx &moveHandler);

    template <RegionFlag REGION_TYPE>
    void ClearCurrentRegion()
    {
        ResetCurrentRegion<false, REGION_TYPE>();
    }

    /**
     * Iterate over specific regions from vector
     * and move all alive objects to the regions with type /param regions_type_to.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param regions - vector of regions needed to proceed.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactSeveralSpecificRegions(const PandaVector<Region *> &regions, const GCObjectVisitor &deathChecker,
                                       const ObjectVisitorEx &moveHandler);

    /**
     * Iterate over specific region
     * and move all alive objects to the regions with type /param regions_type_to.
     * @tparam regions_type_from - type of regions needed to proceed.
     * @tparam regions_type_to - type of regions to which we want to move all alive objects.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @param region - region needed to proceed.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param move_handler - called for every moved object
     *  can be used as a simple visitor if we enable /param use_marked_bitmap
     */
    template <RegionFlag REGIONS_TYPE_FROM, RegionFlag REGIONS_TYPE_TO, bool USE_MARKED_BITMAP = false>
    void CompactSpecificRegion(Region *regions, const GCObjectVisitor &deathChecker,
                               const ObjectVisitorEx &moveHandler);
    /**
     * Promote region and return a counter of moved alive objects during MixedGC if use_marked_bitmap == true, or 0 in
     * any other case scenarios.
     * @tparam use_marked_bitmap - if we need to use marked_bitmap from the regions or not.
     * @tparam full_gc - check it's FullGC or MixedGC.
     * @param region - region needed to proceed.
     * @param death_checker - checker what will return objects status for iterated object.
     * @param alive_objects_handler - called for every alive object.
     */
    template <bool USE_MARKED_BITMAP = false, bool FULL_GC = false>
    size_t PromoteYoungRegion(Region *region, const GCObjectVisitor &deathChecker,
                              const ObjectVisitor &aliveObjectsHandler);

    /**
     * Reset all regions with type /param regions_type.
     * @tparam regions_type - type of regions needed to proceed.
     */
    template <RegionFlag REGIONS_TYPE>
    void ResetAllSpecificRegions();

    /**
     * Reset regions from vector.
     * @tparam REGIONS_TYPE - type of regions needed to proceed.
     * @tparam REGIONS_RELEASE_POLICY - region need to be placed in the free queue or returned to mempool.
     * @tparam OS_PAGES_POLICY - if we need to return region pages to OS or not.
     * @tparam NEED_LOCK - if we need to take region lock or not. Use it if we allocate regions in parallel
     * @param regions - vector of regions needed to proceed.
     * @tparam Container - region's container type
     */
    template <RegionFlag REGIONS_TYPE, RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY,
              OSPagesPolicy OS_PAGES_POLICY, bool NEED_LOCK, typename Container>
    void ResetSeveralSpecificRegions(const Container &regions);

    /// Reserve one region if no reserved region
    void ReserveRegionIfNeeded();

    /// Release reserved region to free space
    void ReleaseReservedRegion();

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &memVisitor)
    {
        this->ClearRegionsPool();
    }

    constexpr static size_t GetMaxRegularObjectSize()
    {
        return REGION_SIZE - AlignUp(sizeof(Region), DEFAULT_ALIGNMENT_IN_BYTES);
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return this->GetSpace()->ContainObject(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        return this->GetSpace()->IsLive(object);
    }

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::REGION_ALLOCATOR;
    }

    void SetDesiredEdenLength(size_t edenLength)
    {
        this->GetSpace()->SetDesiredEdenLength(edenLength);
    }

    void AddPromotedRegionToQueueIfPinned(Region *region)
    {
        if (region->HasPinnedObjects()) {
            ASSERT(region->HasFlag(RegionFlag::IS_PROMOTED));
            PushToRegionQueue<false, RegionFlag::IS_PINNED>(region);
        }
    }

private:
    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    Region *GetCurrentRegion()
    {
        Region **curRegion = GetCurrentRegionPointerUnsafe<REGION_TYPE>();
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (USE_ATOMIC) {
            // Atomic with relaxed order reason: data race with cur_region with no synchronization or ordering
            // constraints imposed on other reads or writes
            return reinterpret_cast<std::atomic<Region *> *>(curRegion)->load(std::memory_order_relaxed);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        }
        return *curRegion;
    }

    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    void SetCurrentRegion(Region *region)
    {
        Region **curRegion = GetCurrentRegionPointerUnsafe<REGION_TYPE>();
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (USE_ATOMIC) {
            // Atomic with relaxed order reason: data race with cur_region with no synchronization or ordering
            // constraints imposed on other reads or writes
            reinterpret_cast<std::atomic<Region *> *>(curRegion)->store(region, std::memory_order_relaxed);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            *curRegion = region;
        }
    }

    template <RegionFlag REGION_TYPE>
    Region **GetCurrentRegionPointerUnsafe()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            return &edenCurrentRegion_;
        }
        UNREACHABLE();
        return nullptr;
    }

    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    void ResetCurrentRegion()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            SetCurrentRegion<USE_ATOMIC, REGION_TYPE>(&fullRegion_);
            return;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            os::memory::LockHolder<os::memory::Mutex, USE_ATOMIC> lock(*GetQueueLock<REGION_TYPE>());
            GetRegionQueuePointer<REGION_TYPE>()->clear();
            return;
        }
        UNREACHABLE();
    }

    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    bool IsInCurrentRegion(Region *region)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            return GetCurrentRegion<USE_ATOMIC, REGION_TYPE>() == region;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE != RegionFlag::IS_OLD) {
            LOG(FATAL, ALLOC) << "Region type is neither eden nor old";
        }
        os::memory::LockHolder<os::memory::Mutex, USE_ATOMIC> lock(*GetQueueLock<REGION_TYPE>());
        for (auto i : *GetRegionQueuePointer<REGION_TYPE>()) {
            if (i == region) {
                return true;
            }
        }
        return false;
    }

public:
    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    Region *PopFromRegionQueue()
    {
        PandaVector<Region *> *regionQueue = GetRegionQueuePointer<REGION_TYPE>();
        os::memory::LockHolder<os::memory::Mutex, USE_ATOMIC> lock(*GetQueueLock<REGION_TYPE>());
        if (regionQueue->empty()) {
            return nullptr;
        }
        auto *region = regionQueue->back();
        regionQueue->pop_back();
        return region;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    void PushToRegionQueue(Region *region)
    {
        PandaVector<Region *> *regionQueue = GetRegionQueuePointer<REGION_TYPE>();
        os::memory::LockHolder<os::memory::Mutex, USE_ATOMIC> lock(*GetQueueLock<REGION_TYPE>());
        regionQueue->push_back(region);
    }

    template <bool USE_ATOMIC = true, RegionFlag REGION_TYPE>
    Region *CreateAndSetUpNewRegionWithLock()
    {
        os::memory::LockHolder<LockConfigT, USE_ATOMIC> lock(this->regionLock_);
        Region *regionTo = this->template CreateAndSetUpNewRegion<AllocConfigT>(DEFAULT_REGION_SIZE, REGION_TYPE);
        ASSERT(regionTo != nullptr);
        return regionTo;
    }

private:
    template <RegionFlag REGION_TYPE>
    os::memory::Mutex *GetQueueLock()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD || REGION_TYPE == RegionFlag::IS_PINNED) {
            return &oldQueueLock_;
        }
        UNREACHABLE();
        return nullptr;
    }

    template <RegionFlag REGION_TYPE>
    PandaVector<Region *> *GetRegionQueuePointer()
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REGION_TYPE == RegionFlag::IS_OLD) {
            return &oldRegionQueue_;
        } else if constexpr (REGION_TYPE == RegionFlag::IS_PINNED) {
            return &pinnedRegionQueue_;
        }
        UNREACHABLE();
        return nullptr;
    }

    template <RegionFlag REGION_TYPE>
    struct RegionMem AllocRegular(size_t alignSize);
    TLAB *CreateTLABInRegion(Region *region, size_t size);

    Region fullRegion_;
    Region *edenCurrentRegion_;
    Region *reservedRegion_ = nullptr;
    os::memory::Mutex oldQueueLock_;
    PandaVector<Region *> oldRegionQueue_;
    PandaVector<Region *> pinnedRegionQueue_;
    // To store partially used Regions that can be reused later.
    ark::PandaMultiMap<size_t, Region *, std::greater<size_t>> retainedTlabs_;
    friend class test::RegionAllocatorTest;
};

template <typename AllocConfigT, typename LockConfigT, typename ObjectAllocator>
class RegionNonmovableAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionNonmovableAllocator);
    NO_COPY_SEMANTIC(RegionNonmovableAllocator);

    explicit RegionNonmovableAllocator(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType,
                                       size_t initSpaceSize = 0, bool extend = true);
    explicit RegionNonmovableAllocator(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType,
                                       RegionPool *sharedRegionPool);

    ~RegionNonmovableAllocator() override = default;

    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void Collect(const GCObjectVisitor &deathChecker);

    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &objVisitor)
    {
        objectAllocator_.IterateOverObjects(objVisitor);
    }

    template <typename MemVisitor>
    void IterateOverObjectsInRange(const MemVisitor &memVisitor, void *begin, void *end)
    {
        objectAllocator_.IterateOverObjectsInRange(memVisitor, begin, end);
    }

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &memVisitor)
    {
        objectAllocator_.VisitAndRemoveAllPools([this](void *mem, [[maybe_unused]] size_t size) {
            auto *region = AddrToRegion(mem);
            ASSERT(ToUintPtr(mem) + size == region->End());
            this->GetSpace()->FreeRegion(region);
        });
    }

    void VisitAndRemoveFreeRegions(const RegionsVisitor &regionVisitor);

    constexpr static size_t GetMaxSize()
    {
        // NOTE(yxr) : get accurate max payload size in a freelist pool
        return std::min(ObjectAllocator::GetMaxSize(), static_cast<size_t>(REGION_SIZE - 1_KB));
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return objectAllocator_.ContainObject(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        ASSERT(this->GetRegion(object)->GetLiveBitmap() != nullptr);
        return this->GetRegion(object)->GetLiveBitmap()->AtomicTest(const_cast<ObjectHeader *>(object));
    }

    double CalculateExternalFragmentation()
    {
        return objectAllocator_.CalculateExternalFragmentation();
    }

    double CalculateDeadObjectsRatio() override;

private:
    void *NewRegionAndRetryAlloc(size_t objectSize, Alignment align);

    mutable ObjectAllocator objectAllocator_;
};

/// @brief A region-based humongous allocator.
template <typename AllocConfigT, typename LockConfigT = RegionAllocatorLockConfig::CommonLock>
class RegionHumongousAllocator final : public RegionAllocatorBase<LockConfigT> {
public:
    static constexpr size_t REGION_SIZE = DEFAULT_REGION_SIZE;

    NO_MOVE_SEMANTIC(RegionHumongousAllocator);
    NO_COPY_SEMANTIC(RegionHumongousAllocator);

    /**
     * @brief Create new humongous region allocator
     * @param mem_stats - memory statistics
     * @param space_type - space type
     */
    explicit RegionHumongousAllocator(MemStatsType *memStats, GenerationalSpaces *spaces, SpaceType spaceType);

    ~RegionHumongousAllocator() override = default;

    template <bool UPDATE_MEMSTATS = true>
    void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    template <typename T>
    T *AllocArray(size_t arrLength)
    {
        return static_cast<T *>(Alloc(sizeof(T) * arrLength));
    }

    void Free([[maybe_unused]] void *mem) {}

    void CollectAndRemoveFreeRegions(const RegionsVisitor &regionVisitor, const GCObjectVisitor &deathChecker);

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @param visitor - function pointer or functor
     */
    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &visitor)
    {
        this->GetSpace()->IterateRegions([&](Region *region) { region->IterateOverObjects(visitor); });
    }

    template <typename ObjectVisitor>
    void IterateOverObjectsInRange(const ObjectVisitor &visitor, void *begin, void *end)
    {
        this->GetSpace()->IterateRegions([&](Region *region) {
            if (!region->Intersect(ToUintPtr(begin), ToUintPtr(end))) {
                return;
            }
            region->IterateOverObjects([&visitor, begin, end](ObjectHeader *obj) {
                if (ToUintPtr(begin) <= ToUintPtr(obj) && ToUintPtr(obj) < ToUintPtr(end)) {
                    visitor(obj);
                }
            });
        });
    }

    void VisitAndRemoveAllPools([[maybe_unused]] const MemVisitor &memVisitor)
    {
        this->ClearRegionsPool();
    }

    bool ContainObject(const ObjectHeader *object) const
    {
        return this->GetSpace()->template ContainObject<true>(object);
    }

    bool IsLive(const ObjectHeader *object) const
    {
        return this->GetSpace()->template IsLive<true>(object);
    }

    double CalculateInternalFragmentation();

private:
    void ResetRegion(Region *region);
    void Collect(Region *region, const GCObjectVisitor &deathChecker);

    // If we change this constant, we will increase fragmentation dramatically
    static_assert(REGION_SIZE / PANDA_POOL_ALIGNMENT_IN_BYTES == 1);
    friend class test::RegionAllocatorTest;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_REGION_ALLOCATOR_H
