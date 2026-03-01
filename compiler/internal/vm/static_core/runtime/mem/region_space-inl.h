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
#ifndef PANDA_RUNTIME_MEM_REGION_SPACE_INL_H
#define PANDA_RUNTIME_MEM_REGION_SPACE_INL_H

#include "runtime/mem/region_space.h"
#include "libarkbase/utils/asan_interface.h"

namespace ark::mem {

class RegionAllocCheck {
public:
    explicit RegionAllocCheck(Region *region) : region_(region)
    {
        ASSERT(region_->SetAllocating(true));
    }
    ~RegionAllocCheck()
    {
        ASSERT(region_->SetAllocating(false));
    }
    NO_COPY_SEMANTIC(RegionAllocCheck);
    NO_MOVE_SEMANTIC(RegionAllocCheck);

private:
    Region *region_ FIELD_UNUSED;
};

class RegionIterateCheck {
public:
    explicit RegionIterateCheck(Region *region) : region_(region)
    {
        ASSERT(region_->SetIterating(true));
    }
    ~RegionIterateCheck()
    {
        ASSERT(region_->SetIterating(false));
    }
    NO_COPY_SEMANTIC(RegionIterateCheck);
    NO_MOVE_SEMANTIC(RegionIterateCheck);

private:
    Region *region_ FIELD_UNUSED;
};

template <bool ATOMIC>
void *Region::Alloc(size_t alignedSize)
{
    RegionAllocCheck alloc(this);
    ASSERT(AlignUp(alignedSize, DEFAULT_ALIGNMENT_IN_BYTES) == alignedSize);
    ASSERT(!IsTLAB() || IsMixedTLAB());
    uintptr_t oldTop;
    uintptr_t newTop;
    if (ATOMIC) {
        auto atomicTop = reinterpret_cast<std::atomic<uintptr_t> *>(&top_);
        // Atomic with relaxed order reason: data race with top_ with no synchronization or ordering constraints imposed
        // on other reads or writes
        oldTop = atomicTop->load(std::memory_order_relaxed);
        do {
            newTop = oldTop + alignedSize;
            if (UNLIKELY(newTop > end_)) {
                return nullptr;
            }
        } while (!atomicTop->compare_exchange_weak(oldTop, newTop, std::memory_order_relaxed));
        ASAN_UNPOISON_MEMORY_REGION(ToVoidPtr(oldTop), alignedSize);
        return ToVoidPtr(oldTop);
    }
    newTop = top_ + alignedSize;
    if (UNLIKELY(newTop > end_)) {
        return nullptr;
    }
    oldTop = top_;
    top_ = newTop;

    ASAN_UNPOISON_MEMORY_REGION(ToVoidPtr(oldTop), alignedSize);
    return ToVoidPtr(oldTop);
}

inline void Region::UndoAlloc(void *addr)
{
    RegionAllocCheck alloc(this);
    ASSERT(HasFlag(RegionFlag::IS_OLD));
    top_ = ToUintPtr(addr);
}

template <typename ObjectVisitor>
void Region::IterateOverObjects(const ObjectVisitor &visitor)
{
    // This method doesn't work for nonmovable regions
    ASSERT(!HasFlag(RegionFlag::IS_NONMOVABLE));
    // currently just for gc stw phase, so check it is not in allocating state
    RegionIterateCheck iterate(this);
    if (!IsTLAB()) {
        auto curPtr = Begin();
        auto endPtr = Top();
        while (curPtr < endPtr) {
            auto objectHeader = reinterpret_cast<ObjectHeader *>(curPtr);
            size_t objectSize = GetObjectSize(objectHeader);
            visitor(objectHeader);
            curPtr = AlignUp(curPtr + objectSize, DEFAULT_ALIGNMENT_IN_BYTES);
        }
    } else {
        for (auto i : *tlabVector_) {
            i->IterateOverObjects(visitor);
        }
        if (IsMixedTLAB()) {
            auto curPtr = ToUintPtr(GetLastTLAB()->GetEndAddr());
            auto endPtr = Top();
            while (curPtr < endPtr) {
                auto *objectHeader = reinterpret_cast<ObjectHeader *>(curPtr);
                size_t objectSize = GetObjectSize(objectHeader);
                visitor(objectHeader);
                curPtr = AlignUp(curPtr + objectSize, DEFAULT_ALIGNMENT_IN_BYTES);
            }
        }
    }
}

template <OSPagesPolicy OS_PAGES_POLICY>
void RegionPool::FreeRegion(Region *region)
{
    bool releasePages = OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN;
    if (block_.IsAddrInRange(region)) {
        region->IsYoung() ? spaces_->ReduceYoungOccupiedInSharedPool(region->Size())
                          : spaces_->ReduceTenuredOccupiedInSharedPool(region->Size());
        block_.FreeRegion(region, releasePages);
    } else {
        region->IsYoung() ? spaces_->FreeYoungPool(region, region->Size(), releasePages)
                          : spaces_->FreeTenuredPool(region, region->Size(), releasePages);
    }
}

template <RegionSpace::ReleaseRegionsPolicy REGIONS_RELEASE_POLICY, OSPagesPolicy OS_PAGES_POLICY>
void RegionSpace::FreeRegion(Region *region)
{
    ASSERT(region->GetSpace() == this);
    ASAN_POISON_MEMORY_REGION(ToVoidPtr(region->Begin()), region->End() - region->Begin());
    regions_.erase(region->AsListNode());
    if (region->IsYoung()) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        [[maybe_unused]] auto previousRegionsInUse = youngRegionsInUse_.fetch_sub(1, std::memory_order_relaxed);
        ASSERT(previousRegionsInUse > 0);
    }
    region->Destroy();
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (REGIONS_RELEASE_POLICY == ReleaseRegionsPolicy::NoRelease) {
        if (region->IsYoung()) {
            // unlimited
            emptyYoungRegions_.push_back(region->AsListNode());
            return;
        }
        if (region->HasFlag(RegionFlag::IS_OLD) && (emptyTenuredRegions_.size() < emptyTenuredRegionsMaxCount_)) {
            emptyTenuredRegions_.push_back(region->AsListNode());
            return;
        }
    }
    regionPool_->FreeRegion<OS_PAGES_POLICY>(region);
}

template <RegionFlag REGION_TYPE, OSPagesPolicy OS_PAGES_POLICY>
void RegionSpace::ReleaseEmptyRegions()
{
    auto visitor = [this](Region *region) { regionPool_->FreeRegion<OS_PAGES_POLICY>(region); };
    if (IsYoungRegionFlag(REGION_TYPE)) {
        IterateRegionsList(emptyYoungRegions_, visitor);
        emptyYoungRegions_.clear();
    } else {
        IterateRegionsList(emptyTenuredRegions_, visitor);
        emptyTenuredRegions_.clear();
    }
}

template <typename RegionVisitor>
void RegionSpace::IterateRegions(RegionVisitor visitor)
{
    IterateRegionsList(regions_, visitor);
}

template <typename RegionVisitor>
void RegionSpace::IterateRegionsList(DList &regionsList, RegionVisitor visitor)
{
    auto it = regionsList.begin();
    while (it != regionsList.end()) {
        auto *region = Region::AsRegion(&(*it));
        ++it;  // increase before visitor which may remove region
        visitor(region);
    }
}

template <bool CROSS_REGION>
bool RegionSpace::ContainObject(const ObjectHeader *object) const
{
    return GetRegion<CROSS_REGION>(object) != nullptr;
}

template <bool CROSS_REGION>
bool RegionSpace::IsLive(const ObjectHeader *object) const
{
    auto *region = GetRegion<CROSS_REGION>(object);

    // check if the object is live in the range
    return region != nullptr && region->IsInAllocRange(object);
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_REGION_SPACE_INL_H
