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
#ifndef PANDA_RUNTIME_MEM_REGION_SPACE_H
#define PANDA_RUNTIME_MEM_REGION_SPACE_H

#include <atomic>
#include <cstdint>

#include "libarkbase/utils/list.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/tlab.h"
#include "runtime/mem/rem_set.h"
#include "runtime/mem/heap_space.h"

namespace ark::mem {

enum RegionFlag {
    IS_UNUSED = 0U,
    IS_EDEN = 1U,
    IS_SURVIVOR = 1U << 1U,
    IS_OLD = 1U << 2U,
    IS_LARGE_OBJECT = 1U << 3U,
    IS_NONMOVABLE = 1U << 4U,
    IS_TLAB = 1U << 5U,
    IS_COLLECTION_SET = 1U << 6U,
    IS_FREE = 1U << 7U,
    IS_PROMOTED = 1U << 8U,
    IS_RESERVED = 1U << 9U,
    IS_PINNED = 1U << 10U,
    IS_MIXEDTLAB = 1U << 11U,
    IS_ZEROED = 1U << 12U,
    IS_INVALID = 1U << 13U,
};

constexpr bool IsYoungRegionFlag(RegionFlag flag)
{
    return flag == RegionFlag::IS_EDEN || flag == RegionFlag::IS_SURVIVOR;
}

static constexpr size_t DEFAULT_REGION_ALIGNMENT = 256_KB;
static constexpr size_t DEFAULT_REGION_SIZE = DEFAULT_REGION_ALIGNMENT;
static constexpr size_t DEFAULT_REGION_MASK = DEFAULT_REGION_ALIGNMENT - 1;

using RemSetT = RemSet<>;

class RegionSpace;
class Region {
public:
    NO_THREAD_SANITIZE explicit Region(RegionSpace *space, uintptr_t begin, uintptr_t end)
        : space_(space), begin_(begin), end_(end), top_(begin)
    {
    }

    ~Region() = default;

    NO_COPY_SEMANTIC(Region);
    NO_MOVE_SEMANTIC(Region);

    void Destroy();

    RegionSpace *GetSpace()
    {
        return space_;
    }

    uintptr_t Begin() const
    {
        return begin_;
    }

    uintptr_t End() const
    {
        return end_;
    }

    bool Intersect(uintptr_t begin, uintptr_t end) const
    {
        return !(end <= begin_ || end_ <= begin);
    }

    uintptr_t Top() const
    {
        return top_;
    }

    void SetTop(uintptr_t newTop)
    {
        ASSERT(!IsTLAB() || IsMixedTLAB());
        top_ = newTop;
    }

    uint32_t GetLiveBytes() const
    {
        ASSERT(liveBytes_ != nullptr);
        // Atomic with relaxed order reason: load value without concurrency
        auto liveBytes = liveBytes_->load(std::memory_order_relaxed);
        ASSERT(liveBytes <= Size());
        return liveBytes;
    }

    uint32_t GetAllocatedBytes() const;

    double GetFragmentation() const;

    uint32_t GetGarbageBytes() const
    {
        ASSERT(GetAllocatedBytes() >= GetLiveBytes());
        return GetAllocatedBytes() - GetLiveBytes();
    }

    void SetLiveBytes(uint32_t count)
    {
        ASSERT(liveBytes_ != nullptr);
        // Atomic with relaxed order reason: store value without concurrency
        liveBytes_->store(count, std::memory_order_relaxed);
    }

    template <bool ATOMICALLY>
    void AddLiveBytes(uint32_t count)
    {
        ASSERT(liveBytes_ != nullptr);
        if constexpr (ATOMICALLY) {
            // Atomic with seq_cst order reason: store value with concurrency
            liveBytes_->fetch_add(count, std::memory_order_seq_cst);
        } else {
            auto *field = reinterpret_cast<uint32_t *>(liveBytes_);
            *field += count;
        }
    }

    uint32_t CalcLiveBytes() const;

    uint32_t CalcMarkBytes() const;

    MarkBitmap *GetLiveBitmap() const
    {
        return liveBitmap_;
    }

    void IncreaseAllocatedObjects()
    {
        // We can call it from the promoted region
        ASSERT(liveBitmap_ != nullptr);
        allocatedObjects_++;
    }

    size_t UpdateAllocatedObjects()
    {
        size_t aliveCount = GetMarkBitmap()->GetSetBitCount();
        SetAllocatedObjects(aliveCount);
        return aliveCount;
    }

    size_t GetAllocatedObjects()
    {
        ASSERT(HasFlag(RegionFlag::IS_OLD));
        return allocatedObjects_;
    }

    MarkBitmap *GetMarkBitmap() const
    {
        return markBitmap_;
    }

    RemSetT *GetRemSet()
    {
        return remSet_;
    }

    size_t GetRemSetSize() const
    {
        return remSet_->Size();
    }

    void AddFlag(RegionFlag flag)
    {
        flags_ |= flag;
    }

    void RmvFlag(RegionFlag flag)
    {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        flags_ &= ~flag;
    }

    bool HasFlags(RegionFlag flag) const
    {
        return (flags_ & flag) == flag;
    }

    bool HasFlag(RegionFlag flag) const
    {
        return (flags_ & flag) != 0;
    }

    bool IsEden() const
    {
        return HasFlag(IS_EDEN);
    }

    bool IsSurvivor() const
    {
        return HasFlag(RegionFlag::IS_SURVIVOR);
    }

    bool IsYoung() const
    {
        return IsEden() || IsSurvivor();
    }

    bool IsInCollectionSet() const
    {
        return HasFlag(IS_COLLECTION_SET);
    }

    bool IsTLAB() const
    {
        ASSERT((tlabVector_ == nullptr) || (top_ == begin_) || IsMixedTLAB());
        return tlabVector_ != nullptr;
    }

    bool IsMixedTLAB() const
    {
        return HasFlag(RegionFlag::IS_MIXEDTLAB);
    }

    size_t Size() const
    {
        return end_ - ToUintPtr(this);
    }

    void PinObject()
    {
        ASSERT(pinnedObjects_ != nullptr);
        // Atomic with seq_cst order reason: add value with concurrency
        pinnedObjects_->fetch_add(1, std::memory_order_seq_cst);
    }

    void UnpinObject()
    {
        ASSERT(pinnedObjects_ != nullptr);
        // Atomic with seq_cst order reason: sub value with concurrency
        pinnedObjects_->fetch_sub(1, std::memory_order_seq_cst);
    }

    bool HasPinnedObjects() const
    {
        ASSERT(pinnedObjects_ != nullptr);
        // Atomic with seq_cst order reason: load value with concurrency
        return pinnedObjects_->load(std::memory_order_seq_cst) > 0;
    }

    template <bool ATOMIC = true>
    NO_THREAD_SANITIZE void *Alloc(size_t alignedSize);

    NO_THREAD_SANITIZE void UndoAlloc(void *addr);

    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &visitor);

    ObjectHeader *GetLargeObject()
    {
        ASSERT(HasFlag(RegionFlag::IS_LARGE_OBJECT));
        return reinterpret_cast<ObjectHeader *>(Begin());
    }

    bool IsInRange(const ObjectHeader *object) const;

    [[nodiscard]] bool IsInAllocRange(const ObjectHeader *object) const;

    static bool IsAlignment(uintptr_t regionAddr, size_t regionSize)
    {
        ASSERT(regionSize != 0);
        return ((regionAddr - HeapStartAddress()) % regionSize) == 0;
    }

    constexpr static size_t HeadSize()
    {
        return AlignUp(sizeof(Region), DEFAULT_ALIGNMENT_IN_BYTES);
    }

    constexpr static size_t RegionSize(size_t objectSize, size_t regionSize)
    {
        return AlignUp(HeadSize() + objectSize, regionSize);
    }

    static uintptr_t HeapStartAddress()
    {
        return PoolManager::GetMmapMemPool()->GetMinObjectAddress();
    }

    InternalAllocatorPtr GetInternalAllocator();

    void CreateRemSet();

    void SetupAtomics();

    void CreateTLABSupport();

    size_t GetRemainingSizeForTLABs() const;
    TLAB *CreateTLAB(size_t size);

    TLAB *GetLastTLAB() const
    {
        ASSERT(tlabVector_ != nullptr);
        ASSERT(!tlabVector_->empty());
        return tlabVector_->back();
    };

    MarkBitmap *CreateMarkBitmap();
    MarkBitmap *CreateLiveBitmap();

    void SwapMarkBitmap();

    void CloneMarkBitmapToLiveBitmap();

    void SetMarkBit(ObjectHeader *object);

#ifndef NDEBUG
    NO_THREAD_SANITIZE bool IsAllocating()
    {
        // Atomic with acquire order reason: data race with is_allocating_ with dependecies on reads after the load
        // which should become visible
        return reinterpret_cast<std::atomic<bool> *>(&isAllocating_)->load(std::memory_order_acquire);
    }

    NO_THREAD_SANITIZE bool IsIterating()
    {
        // Atomic with acquire order reason: data race with is_iterating_ with dependecies on reads after the load which
        // should become visible
        return reinterpret_cast<std::atomic<bool> *>(&isIterating_)->load(std::memory_order_acquire);
    }

    NO_THREAD_SANITIZE bool SetAllocating(bool value)
    {
        if (IsIterating()) {
            return false;
        }
        // Atomic with release order reason: data race with is_allocating_ with dependecies on writes before the store
        // which should become visible acquire
        reinterpret_cast<std::atomic<bool> *>(&isAllocating_)->store(value, std::memory_order_release);
        return true;
    }

    NO_THREAD_SANITIZE bool SetIterating(bool value)
    {
        if (IsAllocating()) {
            return false;
        }
        // Atomic with release order reason: data race with is_iterating_ with dependecies on writes before the store
        // which should become visible acquire
        reinterpret_cast<std::atomic<bool> *>(&isIterating_)->store(value, std::memory_order_release);
        return true;
    }
#endif

    DListNode *AsListNode()
    {
        return &node_;
    }

    static Region *AsRegion(const DListNode *node)
    {
        return reinterpret_cast<Region *>(ToUintPtr(node) - MEMBER_OFFSET(Region, node_));
    }

private:
    void SetAllocatedObjects(size_t allocatedObjects)
    {
        // We can call it from the promoted region
        ASSERT(liveBitmap_ != nullptr);
        allocatedObjects_ = allocatedObjects;
    }

    DListNode node_;
    RegionSpace *space_;
    uintptr_t begin_;
    uintptr_t end_;
    uintptr_t top_;
    uint32_t flags_ {0};
    size_t allocatedObjects_ {0};
    std::atomic<uint32_t> *liveBytes_ {nullptr};
    std::atomic<uint32_t> *pinnedObjects_ {nullptr};
    MarkBitmap *liveBitmap_ {nullptr};           // records live objects for old region
    MarkBitmap *markBitmap_ {nullptr};           // mark bitmap used in current gc marking phase
    RemSetT *remSet_ {nullptr};                  // remember set(old region -> eden/survivor region)
    PandaVector<TLAB *> *tlabVector_ {nullptr};  // pointer to a vector with thread tlabs associated with this region
#ifndef NDEBUG
    bool isAllocating_ = false;
    bool isIterating_ = false;
#endif
};

inline std::ostream &DumpRegionRange(std::ostream &out, const Region &region)
{
    std::ios_base::fmtflags flags = out.flags();
    static constexpr size_t POINTER_PRINT_WIDTH = 8;
    out << std::hex << "[0x" << std::setw(POINTER_PRINT_WIDTH) << std::setfill('0') << region.Begin() << "-0x"
        << std::setw(POINTER_PRINT_WIDTH) << std::setfill('0') << region.End() << "]";
    out.flags(flags);
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const Region &region)
{
    if (region.HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        out << "H";
    } else if (region.HasFlag(RegionFlag::IS_NONMOVABLE)) {
        out << "NM";
    } else if (region.HasFlag(RegionFlag::IS_OLD)) {
        out << "T";
    } else {
        out << "Y";
    }

    return DumpRegionRange(out, region);
}

// RegionBlock is used for allocate regions from a continuous big memory block
// |--------------------------|
// |.....RegionBlock class....|
// |--------------------------|
// |.......regions_end_.......|--------|
// |.......regions_begin_.....|----|   |
// |--------------------------|    |   |
//                                 |   |
// |   Continuous Mem Block   |    |   |
// |--------------------------|    |   |
// |...........Region.........|<---|   |
// |...........Region.........|        |
// |...........Region.........|        |
// |..........................|        |
// |..........................|        |
// |..........................|        |
// |..........................|        |
// |..........................|        |
// |..........................|        |
// |..........................|        |
// |...........Region.........|<-------|
class RegionBlock {
public:
    RegionBlock(size_t regionSize, InternalAllocatorPtr allocator) : regionSize_(regionSize), allocator_(allocator) {}

    ~RegionBlock()
    {
        if (!occupied_.Empty()) {
            allocator_->Free(occupied_.Data());
        }
    }

    NO_COPY_SEMANTIC(RegionBlock);
    NO_MOVE_SEMANTIC(RegionBlock);

    void Init(uintptr_t regionsBegin, uintptr_t regionsEnd);

    Region *AllocRegion();

    Region *AllocLargeRegion(size_t largeRegionSize);

    void FreeRegion(Region *region, bool releasePages = true);

    bool IsAddrInRange(const void *addr) const
    {
        return ToUintPtr(addr) < regionsEnd_ && ToUintPtr(addr) >= regionsBegin_;
    }

    Region *GetAllocatedRegion(const void *addr) const
    {
        ASSERT(IsAddrInRange(addr));
        os::memory::LockHolder lock(lock_);
        return occupied_[RegionIndex(addr)];
    }

    size_t GetFreeRegionsNum() const
    {
        os::memory::LockHolder lock(lock_);
        return occupied_.Size() - numUsedRegions_;
    }

private:
    Region *RegionAt(size_t index) const
    {
        return reinterpret_cast<Region *>(regionsBegin_ + index * regionSize_);
    }

    size_t RegionIndex(const void *addr) const
    {
        return (ToUintPtr(addr) - regionsBegin_) / regionSize_;
    }

    size_t regionSize_;
    InternalAllocatorPtr allocator_;
    uintptr_t regionsBegin_ = 0;
    uintptr_t regionsEnd_ = 0;
    size_t numUsedRegions_ = 0;
    Span<Region *> occupied_ GUARDED_BY(lock_);
    mutable os::memory::Mutex lock_;
};

// RegionPool supports to work in three ways:
// 1.alloc region in pre-allocated buffer(RegionBlock)
// 2.alloc region in mmap pool directly
// 3.mixed above two ways
class RegionPool {
public:
    explicit RegionPool(size_t regionSize, bool extend, GenerationalSpaces *spaces, InternalAllocatorPtr allocator)
        : block_(regionSize, allocator),
          regionSize_(regionSize),
          spaces_(spaces),
          allocator_(allocator),
          extend_(extend)
    {
    }

    Region *NewRegion(RegionSpace *space, SpaceType spaceType, AllocatorType allocatorType, size_t regionSize,
                      RegionFlag edenOrOldOrNonmovable, RegionFlag properties,
                      OSPagesAllocPolicy allocPolicy = OSPagesAllocPolicy::NO_POLICY);

    Region *NewRegion(void *region, RegionSpace *space, size_t regionSize, RegionFlag edenOrOldOrNonmovable,
                      RegionFlag properties, RegionFlag zeroed = RegionFlag::IS_UNUSED);

    template <OSPagesPolicy OS_PAGES_POLICY = OSPagesPolicy::IMMEDIATE_RETURN>
    void FreeRegion(Region *region);

    void PromoteYoungRegion(Region *region);

    void InitRegionBlock(uintptr_t regionsBegin, uintptr_t regionsEnd)
    {
        block_.Init(regionsBegin, regionsEnd);
    }

    bool IsAddrInPoolRange(const void *addr) const
    {
        return block_.IsAddrInRange(addr) || IsAddrInExtendPoolRange(addr);
    }

    template <bool CROSS_REGION = false>
    Region *GetRegion(const void *addr) const
    {
        if (block_.IsAddrInRange(addr)) {
            return block_.GetAllocatedRegion(addr);
        }
        if (IsAddrInExtendPoolRange(addr)) {
            return AddrToRegion<CROSS_REGION>(addr);
        }
        return nullptr;
    }

    size_t GetFreeRegionsNumInRegionBlock() const
    {
        return block_.GetFreeRegionsNum();
    }

    bool HaveTenuredSize(size_t size) const;

    bool HaveFreeRegions(size_t numRegions, size_t regionSize) const;

    InternalAllocatorPtr GetInternalAllocator()
    {
        return allocator_;
    }

    ~RegionPool() = default;
    NO_COPY_SEMANTIC(RegionPool);
    NO_MOVE_SEMANTIC(RegionPool);

private:
    template <bool CROSS_REGION>
    static Region *AddrToRegion(const void *addr, size_t mask = DEFAULT_REGION_MASK)
    {
        // if it is possible that (object address - region start addr) larger than region alignment,
        // we should get the region start address from mmappool which records it in allocator info
        if constexpr (CROSS_REGION) {  // NOLINT(readability-braces-around-statements, bugprone-suspicious-semicolon)
            ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(addr) == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);

            auto regionAddr = PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(const_cast<void *>(addr));
            return reinterpret_cast<Region *>(regionAddr);
        }
        ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(addr) != SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);

        return reinterpret_cast<Region *>(((ToUintPtr(addr)) & ~mask));
    }

    bool IsAddrInExtendPoolRange(const void *addr) const
    {
        if (extend_) {
            AllocatorInfo allocInfo = PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(const_cast<void *>(addr));
            return allocInfo.GetAllocatorHeaderAddr() == this;
        }
        return false;
    }

    RegionBlock block_;
    size_t regionSize_;
    GenerationalSpaces *spaces_ {nullptr};
    InternalAllocatorPtr allocator_;
    bool extend_ = true;
};

class RegionSpace {
public:
    explicit RegionSpace(SpaceType spaceType, AllocatorType allocatorType, RegionPool *regionPool,
                         size_t emptyTenuredRegionsMaxCount = 0)
        : spaceType_(spaceType),
          allocatorType_(allocatorType),
          regionPool_(regionPool),
          emptyTenuredRegionsMaxCount_(emptyTenuredRegionsMaxCount)
    {
    }

    virtual ~RegionSpace()
    {
        FreeAllRegions();
    }

    NO_COPY_SEMANTIC(RegionSpace);
    NO_MOVE_SEMANTIC(RegionSpace);

    enum class ReleaseRegionsPolicy : bool {
        Release,    // NOLINT(readability-identifier-naming)
        NoRelease,  // NOLINT(readability-identifier-naming)
    };

    Region *NewRegion(size_t regionSize, RegionFlag edenOrOldOrNonmovable, RegionFlag properties,
                      OSPagesAllocPolicy allocPolicy = OSPagesAllocPolicy::NO_POLICY);

    template <ReleaseRegionsPolicy REGIONS_RELEASE_POLICY = ReleaseRegionsPolicy::Release,
              OSPagesPolicy OS_PAGES_POLICY = OSPagesPolicy::IMMEDIATE_RETURN>
    void FreeRegion(Region *region);

    void PromoteYoungRegion(Region *region);

    void FreeAllRegions();

    template <typename RegionVisitor>
    void IterateRegions(RegionVisitor visitor);

    RegionPool *GetPool() const
    {
        return regionPool_;
    }

    template <bool CROSS_REGION = false>
    Region *GetRegion(const ObjectHeader *object) const
    {
        auto *region = regionPool_->GetRegion<CROSS_REGION>(object);

        // check if the region is allocated by this space
        return (region != nullptr && region->GetSpace() == this) ? region : nullptr;
    }

    template <bool CROSS_REGION = false>
    bool ContainObject(const ObjectHeader *object) const;

    template <bool CROSS_REGION = false>
    bool IsLive(const ObjectHeader *object) const;

    template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
    void ReleaseEmptyRegions();

    void SetDesiredEdenLength(size_t edenLength)
    {
        desiredEdenLength_ = edenLength;
    }

private:
    template <typename RegionVisitor>
    void IterateRegionsList(DList &regionsList, RegionVisitor visitor);

    Region *GetRegionFromEmptyList(DList &regionList);

    SpaceType spaceType_;

    // related allocator type
    AllocatorType allocatorType_;

    // underlying shared region pool
    RegionPool *regionPool_;

    size_t emptyTenuredRegionsMaxCount_;

    // region allocated by this space
    DList regions_;

    // Empty regions which is not returned back
    DList emptyYoungRegions_;
    DList emptyTenuredRegions_;
    // Use atomic because it is updated in RegionSpace::PromoteYoungRegion without lock
    std::atomic<size_t> youngRegionsInUse_ {0};
    // Desired eden length is not restricted initially
    size_t desiredEdenLength_ {std::numeric_limits<size_t>::max()};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_REGION_SPACE_H
